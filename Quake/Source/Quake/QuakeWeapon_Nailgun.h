#pragma once

#include "CoreMinimal.h"
#include "QuakeWeaponBase.h"
#include "QuakeWeapon_Nailgun.generated.h"

class AQuakeProjectile;

/**
 * Phase 6 nailgun. SPEC section 2.0 weapon table row 4:
 *   - RoF:              8 shots/sec
 *   - Ammo:             1 nail per shot (EQuakeAmmoType::Nails)
 *   - Damage:           9 per hit (carried on AQuakeProjectile_Nail)
 *   - Projectile:       AQuakeProjectile_Nail (1500 u/s, no gravity)
 *   - Spread half-angle: 1° cone around the aim direction
 *
 * The nailgun uses the same muzzle spawn-out pattern as the Rocket Launcher
 * (SPEC 1.6 rule 1) — the projectile is spawned 60 u in front of the
 * firer's eye viewpoint to stay clear of the capsule's first-frame sweep.
 * The offset is smaller of a concern for nails (which are physical spikes,
 * not an exploding payload) but we keep the same value for consistency and
 * so no weapon in the project has to think about spawn-clipping.
 *
 * Per the project convention, all stats are UPROPERTY defaults set in the
 * C++ constructor. The thin BP_Weapon_Nailgun subclass only fills in the
 * viewmodel mesh + material slots plus ProjectileClass (set to
 * BP_Projectile_Nail) — zero nodes in the BP event graph.
 */
UCLASS()
class QUAKE_API AQuakeWeapon_Nailgun : public AQuakeWeaponBase
{
	GENERATED_BODY()

public:
	AQuakeWeapon_Nailgun();

	/**
	 * Projectile type to spawn on each fire. Set in BP_Weapon_Nailgun to
	 * BP_Projectile_Nail so the asset references resolve at runtime; C++
	 * references the abstract base for uniformity with the Rocket Launcher.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Nailgun")
	TSubclassOf<AQuakeProjectile> ProjectileClass;

	/**
	 * Forward offset from the firer's eye viewpoint at which the nail is
	 * spawned. Matches the Rocket Launcher's SPEC 1.6 rule 1 value (60 u)
	 * for consistency.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Nailgun", meta = (ClampMin = "0.0"))
	float MuzzleSpawnForwardOffset = 60.f;

	/**
	 * Cone half-angle in degrees for nail jitter. SPEC 2.0 column: 1°.
	 * FMath::VRandCone inside Fire uses this to produce a direction
	 * uniformly distributed inside the cone around the aim axis.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Nailgun", meta = (ClampMin = "0.0", ClampMax = "45.0"))
	float SpreadHalfAngleDegrees = 1.f;

	virtual void ApplyStatsFromRow(const struct FQuakeWeaponStatsRow& Row) override;

protected:
	virtual bool CanActuallyFire(AActor* InInstigator) const override;
	virtual void Fire(AActor* InInstigator) override;
};
