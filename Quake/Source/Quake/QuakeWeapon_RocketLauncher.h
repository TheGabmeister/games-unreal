#pragma once

#include "CoreMinimal.h"
#include "QuakeWeaponBase.h"
#include "QuakeWeapon_RocketLauncher.generated.h"

class AQuakeProjectile;

/**
 * Phase 5 rocket launcher. SPEC section 2.0 weapon table row 7:
 *   - RoF:              1.5 shots/sec
 *   - Ammo:             1 rocket per shot (EQuakeAmmoType::Rockets)
 *   - Damage:           100 direct + 120 u linear-falloff splash
 *                       (both carried on AQuakeProjectile_Rocket)
 *   - Projectile:       AQuakeProjectile_Rocket (1000 u/s, no gravity)
 *
 * **Muzzle spawn-out (SPEC 1.6 rule 1).** The projectile is spawned 60 u in
 * front of the firer's eye viewpoint, not at the pawn origin. Without this
 * the first-frame sphere sweep inside the firer's capsule detonates the
 * rocket instantly. AQuakeProjectile::BeginPlay adds a second guard via
 * IgnoreActorWhenMoving(Instigator, true) so a pathological aim angle
 * can't bypass this even if the 60 u cushion is reduced.
 *
 * Per the project convention, all stats are UPROPERTY defaults set in the
 * C++ constructor. The thin BP_Weapon_RocketLauncher subclass only fills
 * in the viewmodel mesh + material slots plus ProjectileClass (set to
 * BP_Projectile_Rocket) — zero nodes in the BP event graph.
 */
UCLASS()
class QUAKE_API AQuakeWeapon_RocketLauncher : public AQuakeWeaponBase
{
	GENERATED_BODY()

public:
	AQuakeWeapon_RocketLauncher();

	/**
	 * Projectile type to spawn on each fire. Set in BP_Weapon_RocketLauncher
	 * to BP_Projectile_Rocket so the asset references resolve at runtime;
	 * C++ references the abstract base so later phases can reuse the same
	 * slot pattern for the Grenade Launcher (BP_Projectile_Grenade).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Rocket")
	TSubclassOf<AQuakeProjectile> ProjectileClass;

	/**
	 * Forward offset from the firer's eye viewpoint at which the projectile
	 * is spawned. SPEC 1.6 rule 1 specifies 60 u — the minimum distance that
	 * prevents first-frame self-detonation against the capsule.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Rocket", meta = (ClampMin = "0.0"))
	float MuzzleSpawnForwardOffset = 60.f;

	virtual void ApplyStatsFromRow(const struct FQuakeWeaponStatsRow& Row) override;

protected:
	virtual bool CanActuallyFire(AActor* InInstigator) const override;
	virtual void Fire(AActor* InInstigator) override;
};
