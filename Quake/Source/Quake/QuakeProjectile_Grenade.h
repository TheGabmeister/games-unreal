#pragma once

#include "CoreMinimal.h"
#include "QuakeProjectile.h"
#include "QuakeProjectile_Grenade.generated.h"

/**
 * Phase 7 grenade projectile. SPEC section 2.0 weapon table row 6 +
 * section 2.2 grenade lifecycle:
 *
 *   - Speed:        800 u/s, arcing (gravity 1.0)
 *   - Bounce:       restitution 0.4 on world geometry
 *   - Fuse:         2.5 s from spawn — never reset by bouncing
 *   - Direct damage:100 + 120 u linear-falloff splash
 *   - Damage type:  UQuakeDamageType_Explosive (same as Rocket)
 *
 * Three detonation conditions (first to occur wins):
 *   1. Fuse timer expires (2.5 s).
 *   2. OnComponentHit with a Pawn-channel actor (player or enemy).
 *   3. Firer's own pawn collides after the 0.25 s grace period.
 *
 * Bouncing off world geometry triggers OnProjectileBounce (sound stub)
 * but does NOT detonate.
 */
UCLASS()
class QUAKE_API AQuakeProjectile_Grenade : public AQuakeProjectile
{
	GENERATED_BODY()

public:
	AQuakeProjectile_Grenade();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Grenade", meta = (ClampMin = "0.0"))
	float BaseDamage = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Grenade", meta = (ClampMin = "0.0"))
	float SplashRadius = 120.f;

	/** Seconds after spawn before the grenade detonates. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Grenade", meta = (ClampMin = "0.0"))
	float FuseTime = 2.5f;

	/** Grace period after spawn during which the firer is still ignored. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Grenade", meta = (ClampMin = "0.0"))
	float FirerGracePeriod = 0.25f;

protected:
	virtual void BeginPlay() override;
	virtual void HandleImpact(const FHitResult& Hit, AActor* OtherActor) override;

private:
	/** Fuse callback — explodes at the grenade's current location. */
	void OnFuseExpired();

	/** Shared explosion logic used by both fuse and contact detonation. */
	void Explode();

	/** Bounce sound stub — wired to OnProjectileBounce in the ctor. */
	UFUNCTION()
	void OnGrenadeBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	/** World time at which the firer grace period ends. */
	double FirerGraceEndTime = 0.0;

	FTimerHandle FuseTimerHandle;
};
