#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "QuakeBalanceRows.generated.h"

/**
 * DataTable row for per-enemy-type balance stats. One row per enemy
 * (Grunt, Knight, Ogre, etc.), keyed by FName. Maps 1:1 to the SPEC
 * section 3.1 stat table. See UQuakeProjectSettings::EnemyStatsTable.
 */
USTRUCT(BlueprintType)
struct FQuakeEnemyStatsRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception")
	float SightRadius = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception")
	float LoseSightRadius = 2200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception")
	float PeripheralVisionAngleDegrees = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception")
	float HearingRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception")
	float SightMaxAge = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float AttackRange = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float AttackDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float AttackCooldown = 1.5f;
};

/**
 * DataTable row for per-weapon balance stats. One row per weapon
 * (Axe, Shotgun, Nailgun, RocketLauncher, etc.), keyed by FName.
 * Maps to the SPEC section 2.0 weapon table. Fields that don't apply
 * to a given weapon type default to zero/one and are ignored by that
 * weapon's ApplyStatsFromRow override.
 *
 * Damage semantics: for single-hit weapons (Axe) this is total damage;
 * for multi-pellet weapons (Shotgun) this is damage per pellet. For
 * projectile weapons (Nailgun, Rocket Launcher) damage lives on the
 * projectile actor, not the weapon — the field is informational for
 * designers and is not read by weapon code.
 */
USTRUCT(BlueprintType)
struct FQuakeWeaponStatsRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire")
	float RateOfFire = 2.f;

	/** Total damage (single-hit) or damage-per-pellet (multi-pellet). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float Damage = 20.f;

	/** Hitscan trace range. 0 for projectile weapons. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float Range = 64.f;

	/** Pellets per shot. 1 for non-shotgun weapons. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shotgun")
	int32 PelletCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spread")
	float SpreadHalfAngleDegrees = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	float MuzzleSpawnForwardOffset = 60.f;

	/** Splash radius. Lives on the projectile actor at runtime, but
	 *  included here so designers see the full weapon picture in one table.
	 *  Not read by weapon code — see AQuakeProjectile_Rocket::SplashRadius. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Splash")
	float SplashRadius = 0.f;
};
