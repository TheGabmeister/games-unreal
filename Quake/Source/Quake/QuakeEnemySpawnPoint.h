#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuakeActivatable.h"
#include "QuakeDifficulty.h"
// Full include needed for TSubclassOf<AQuakeEnemyBase> — same gotcha as
// TSubclassOf<AQuakeWeaponBase> in QuakeGameInstance.h. The include must
// appear before the .generated.h or UHT rejects it.
#include "QuakeEnemyBase.h"
#include "QuakeEnemySpawnPoint.generated.h"

/**
 * Phase 9 spawn-point marker per SPEC section 5.1. The **only** authoring
 * path for enemies that count toward KillsTotal and the level-clear check:
 * directly-placed BP_Enemy_* actors are treated as decoration.
 *
 * Implements IQuakeActivatable so buttons and relays (SPEC 5.5/5.6) can
 * fire a deferred spawn point the same way they fire doors and secrets.
 * AQuakeTrigger_Spawn holds a typed array for editor picker filtering.
 *
 * Lifecycle:
 *   - bDeferredSpawn == false: TrySpawn fires in BeginPlay.
 *   - bDeferredSpawn == true : TrySpawn fires on Activate().
 *
 * TrySpawn is idempotent via the SpawnedEnemy pointer — once populated,
 * subsequent Activate calls are no-ops. A spawn point spawns at most once
 * per level attempt.
 */
UCLASS()
class QUAKE_API AQuakeEnemySpawnPoint : public AActor, public IQuakeActivatable
{
	GENERATED_BODY()

public:
	AQuakeEnemySpawnPoint();

	/** Enemy class to spawn at this point (typically a BP_Enemy_* subclass). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<AQuakeEnemyBase> EnemyClass;

	/**
	 * Lowest difficulty at which this spawn point activates. Easy = always
	 * on, Nightmare = Nightmare-only. Compared via `CurrentDifficulty >=
	 * MinDifficulty`, which the enum's ordering guarantees.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	EQuakeDifficulty MinDifficulty = EQuakeDifficulty::Easy;

	/**
	 * If true, wait for Activate() (e.g. from AQuakeTrigger_Spawn or a
	 * button chain) before spawning. If false, spawn in BeginPlay.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	bool bDeferredSpawn = false;

	/**
	 * If true, this spawn point's enemy counts toward KillsTotal and the
	 * level-clear check. Default true — flip to false for ambient / boss-
	 * arena decoration that shouldn't gate exit unlock.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	bool bIsMarkedKillTarget = true;

	/** Pointer to the spawned pawn. Populated by TrySpawn; null until spawned. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<AQuakeEnemyBase> SpawnedEnemy;

	/**
	 * True if this spawn point participates in stat / clear counting at the
	 * given difficulty. Pure helper so tests can exercise the predicate
	 * without spinning up a GameMode. The Difficulty-less overload pulls
	 * the current difficulty from AQuakeGameMode::GetDifficulty; when the
	 * GameMode is unreachable it falls back to Normal so tests don't fail
	 * on null worlds.
	 */
	UFUNCTION(BlueprintPure, Category = "Spawn")
	bool IsEligible() const;
	bool IsEligibleForDifficulty(EQuakeDifficulty Current) const;

	/**
	 * True when the spawned enemy exists and is dead. Unspawned eligible
	 * spawn points are NOT satisfied — a deferred trigger gate still owes
	 * its enemy to the level-clear count. Forward-compat for Zombies:
	 * IsDead() returns false during the Down state and true only on
	 * permanent death, so a Down zombie correctly keeps the level unclear.
	 */
	UFUNCTION(BlueprintPure, Category = "Spawn")
	bool IsSatisfied() const;

	// IQuakeActivatable
	virtual void Activate(AActor* InInstigator) override;

	virtual void BeginPlay() override;

protected:
	/**
	 * Spawn EnemyClass at our transform, stamp the spawned pawn with a
	 * back-pointer so its Die() path can credit the player on kill, and
	 * store it in SpawnedEnemy. Bails if not eligible, already spawned, or
	 * EnemyClass is null. Returns the spawned pawn or nullptr.
	 */
	AQuakeEnemyBase* TrySpawn();
};
