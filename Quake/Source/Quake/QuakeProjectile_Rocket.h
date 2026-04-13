#pragma once

#include "CoreMinimal.h"
#include "QuakeProjectile.h"
#include "QuakeProjectile_Rocket.generated.h"

/**
 * Phase 5 rocket projectile. SPEC section 2.0 weapon table row 7:
 *   - Speed:          1000 u/s straight (no gravity)
 *   - Direct damage:  100   (applied implicitly by the splash — the victim
 *                     is inside the 0 u inner radius so they take full
 *                     BaseDamage from ApplyRadialDamageWithFalloff)
 *   - Splash radius:  120 u (SPEC 2.2)
 *   - Splash falloff: linear — full at center, 0 at radius edge
 *   - Damage type:    UQuakeDamageType_Explosive
 *                     (SelfDamageScale 0.5, KnockbackScale 4.0)
 *
 * Per SPEC section 1.5 the splash RADIUS is carried here on the projectile,
 * NOT on UQuakeDamageType_Explosive — that type is a shared stateless tag.
 * The rocket is the thing that knows where it exploded, so the radius and
 * BaseDamage live with it as UPROPERTY defaults.
 *
 * Per the project convention, all stats are UPROPERTY defaults set in the
 * C++ constructor. The thin BP_Projectile_Rocket subclass only fills in the
 * mesh + material asset slots — zero nodes in the BP event graph.
 */
UCLASS()
class QUAKE_API AQuakeProjectile_Rocket : public AQuakeProjectile
{
	GENERATED_BODY()

public:
	AQuakeProjectile_Rocket();

	/** Damage at the center of the explosion. SPEC: 100. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Rocket", meta = (ClampMin = "0.0"))
	float BaseDamage = 100.f;

	/**
	 * Full-damage plateau radius (unreal units). Everything within this
	 * distance of the explosion takes full BaseDamage; falloff is linear
	 * from DamageInnerRadius to SplashRadius. Non-zero to work around
	 * UE's ApplyRadialDamageWithFalloff measuring distance to the victim's
	 * component-location (capsule center), not the capsule surface — a
	 * direct hit on a 35 u / 90 u capsule reports ~37 u of distance and
	 * only ~69 damage without this plateau. See CLAUDE.md "Radial damage
	 * measures to component center, not surface" for the full explanation.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Rocket", meta = (ClampMin = "0.0"))
	float DamageInnerRadius = 60.f;

	/** Outer radius of the splash in unreal units. SPEC 2.2: 120. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Rocket", meta = (ClampMin = "0.0"))
	float SplashRadius = 120.f;

	/**
	 * Pure static helper: linear falloff damage at a given distance from the
	 * explosion center. Extracted so the formula can be unit-tested without
	 * spinning up a world, the same pattern as
	 * AQuakeCharacter::ApplyArmorAbsorption,
	 * UQuakeCharacterMovementComponent::ApplyQuakeAirAccel, and
	 * AQuakeEnemyBase::ComputePainChance.
	 *
	 * Returns InBaseDamage at Distance == 0, 0 at Distance >= InRadius, and a
	 * linear interpolation in between:
	 *
	 *     damage = BaseDamage * (1 - Distance / Radius)   (0 <= d < r)
	 *     damage = 0                                       (d >= r)
	 *
	 * Mirrors UGameplayStatics::ApplyRadialDamageWithFalloff with InnerRadius
	 * = 0, DamageFalloff = 1.0 (the parameters HandleImpact passes below).
	 */
	static float ComputeLinearFalloffDamage(float InBaseDamage, float InDistance, float InRadius);

protected:
	virtual void HandleImpact(const FHitResult& Hit, AActor* OtherActor) override;
};
