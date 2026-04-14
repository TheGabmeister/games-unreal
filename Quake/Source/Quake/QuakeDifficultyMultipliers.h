#pragma once

#include "CoreMinimal.h"
#include "QuakeDifficultyMultipliers.generated.h"

/**
 * Phase 12 per-difficulty scaling coefficients per DESIGN 6.1.
 *
 * Applied by AQuakeEnemyBase::ApplyDifficultyScaling in BeginPlay:
 *   MaxHealth              = BaseMaxHealth * EnemyHP
 *   AttackDamageMultiplier = EnemyDamage      (applied at outgoing fire)
 *
 * Spawn-count scaling is NOT a multiplier (see DESIGN 6.1) — it's
 * per-placement MinDifficulty filtering on AQuakeEnemySpawnPoint.
 *
 * Authored as a BP-tunable TMap<EQuakeDifficulty, FQuakeDifficultyMultipliers>
 * on AQuakeGameMode; C++ seeds the SPEC 6.1 defaults in the constructor so
 * an unconfigured BP still plays correctly.
 */
USTRUCT(BlueprintType)
struct FQuakeDifficultyMultipliers
{
	GENERATED_BODY()

	/** Enemy outgoing damage scale. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float EnemyDamage = 1.f;

	/** Enemy MaxHealth scale — baked into MaxHealth at BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float EnemyHP = 1.f;

	/** True iff enemies are pain-immune at this difficulty (SPEC 6.1: Nightmare). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSuppressPain = false;

	/** Zombie revive-timer scale (reserved for v2 — Zombies don't ship in v1). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float ZombieReviveScale = 1.f;
};
