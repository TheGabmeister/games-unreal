#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuakeProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

/**
 * Phase 5 projectile base. SPEC section 2.2 "Projectile implementation":
 * every Quake projectile (Rocket / Nail / Grenade) is an AActor carrying a
 * USphereComponent (collision) + UProjectileMovementComponent (flight). All
 * three share this base; leaf subclasses only tune the collision radius,
 * ProjectileMovement properties, and the HandleImpact override.
 *
 * **Muzzle spawn-out (SPEC 1.6 rule 1).** Rockets must not self-detonate on
 * the firing pawn's capsule on frame 1. Two guards cooperate:
 *   1. The weapon spawns the projectile 60 u in front of the pawn, not at
 *      the muzzle itself (applied at the call site in the weapon's Fire).
 *   2. BeginPlay here calls IgnoreActorWhenMoving(Instigator, true) as a
 *      belt-and-braces second guard so a sweep that happens to start inside
 *      the firer (e.g. tight wall corner) still can't register a hit
 *      against them. The ignore is left in place for the projectile's
 *      lifetime since rockets should never bounce off the firer mid-flight
 *      either.
 *
 * **Collision responses (SPEC 1.6 object response matrix, "Projectile sphere"
 * row).** Object type = Projectile. Block WorldStatic, WorldDynamic, Pawn,
 * Corpse. Ignore Pickup, Projectile, Visibility, Weapon trace. Set up in
 * this base ctor so every subclass inherits the right matrix.
 *
 * **Hit delivery.** The sphere's OnComponentHit delegate is bound in the
 * ctor and forwards to the virtual HandleImpact(Hit, OtherActor) hook.
 * Subclasses (Rocket, Nail, Grenade) override HandleImpact to apply damage
 * and decide when to destroy — the base does not Destroy automatically
 * because grenades want to bounce, not explode, on world hits.
 *
 * Per the project convention, gameplay stats live as UPROPERTY defaults in
 * the C++ subclass constructor. The thin BP subclasses (BP_Projectile_*)
 * only fill in mesh + material + light asset slots — zero event graph
 * nodes.
 */
UCLASS(Abstract)
class QUAKE_API AQuakeProjectile : public AActor
{
	GENERATED_BODY()

public:
	AQuakeProjectile();

	/** Sphere is the root + collision. Subclasses tune radius in their ctor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionSphere;

	/** Visual primitive; BP subclass fills in the mesh + material slots. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	/** Flight. Subclasses set speed, gravity scale, bounce, etc. in their ctor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/**
	 * Multiplier on the projectile's damage output at impact. Set by the
	 * firing weapon's Fire() to AQuakeCharacter::GetOutgoingDamageScale() at
	 * spawn time so Quad (4×) is baked onto the shot, not re-sampled on
	 * hit. Default 1.0 for non-powered-up shots and for enemy projectiles
	 * (Ogre grenades) — enemies don't read PlayerState.
	 *
	 * SPEC 4.3: "does not affect splash self-damage scale". That requirement
	 * is satisfied automatically — the self-damage scale is pulled off
	 * UQuakeDamageType inside TakeDamage, so it multiplies on top of
	 * whatever DamageScale produced here.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile", meta = (ClampMin = "0.0"))
	float DamageScale = 1.f;

protected:
	virtual void BeginPlay() override;

	/**
	 * Subclass hook: called by the sphere's OnComponentHit delegate when the
	 * projectile hits a blocking object. Default implementation is empty —
	 * subclasses decide whether to apply point damage, spawn a splash, and
	 * whether to Destroy() or keep living (grenades bounce instead).
	 *
	 * @param Hit          Detailed hit info for the impact.
	 * @param OtherActor   The actor we hit (may be null for world geometry).
	 */
	virtual void HandleImpact(const FHitResult& Hit, AActor* OtherActor);

private:
	UFUNCTION()
	void OnSphereHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);
};
