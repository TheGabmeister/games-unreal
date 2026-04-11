#pragma once

#include "CoreMinimal.h"
#include "QuakeWeaponBase.h"
#include "QuakeWeapon_Shotgun.generated.h"

/**
 * Phase 4 hitscan shotgun. SPEC section 2.0 weapon table row 2:
 *   - Damage per pellet: 4  (6 pellets -> 24 max per shot)
 *   - Pellet count:      6
 *   - Spread cone:       4° half-angle
 *   - Range:             4096 units (effectively hitscan)
 *   - RoF:               1.5 shots/sec
 *   - Ammo:              1 shell per shot
 *
 * Fires six independent traces from the pawn's eyes, each jittered inside
 * a 4° cone around the aim direction. Each hit calls ApplyPointDamage
 * individually, so six pellets on a Grunt = six TakeDamage calls with
 * Bullet damage type. Knockback stacks across pellets naturally via the
 * per-hit knockback formula in AQuakeCharacter::TakeDamage.
 *
 * Per the project convention, all stats are UPROPERTY defaults set in the
 * C++ constructor. The thin BP_Weapon_Shotgun subclass only fills viewmodel
 * mesh + material slots — zero nodes in the BP event graph.
 */
UCLASS()
class QUAKE_API AQuakeWeapon_Shotgun : public AQuakeWeaponBase
{
	GENERATED_BODY()

public:
	AQuakeWeapon_Shotgun();

	/** Damage applied per pellet trace that hits. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Shotgun", meta = (ClampMin = "0.0"))
	float DamagePerPellet = 4.f;

	/** Number of pellets per shot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Shotgun", meta = (ClampMin = "1"))
	int32 PelletCount = 6;

	/** Cone half-angle in degrees for pellet jitter. SPEC: 4°. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Shotgun", meta = (ClampMin = "0.0", ClampMax = "45.0"))
	float SpreadHalfAngleDegrees = 4.f;

	/** Maximum trace range in unreal units. SPEC: 4096. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Shotgun", meta = (ClampMin = "0.0"))
	float Range = 4096.f;

protected:
	virtual void Fire(AActor* InInstigator) override;
};
