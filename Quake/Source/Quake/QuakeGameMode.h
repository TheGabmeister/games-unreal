#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "QuakeDifficulty.h"
#include "QuakeGameMode.generated.h"

class AQuakeEnemySpawnPoint;

/**
 * Per SPEC section 5.9 and [CLAUDE.md](CLAUDE.md) "Architecture: State
 * Ownership", the GameMode owns the level's **denominators** — KillsTotal,
 * SecretsTotal — computed once at BeginPlay and never updated afterward.
 * The matching **numerators** (Kills, Secrets, TimeElapsed, Deaths) live
 * on AQuakePlayerState as the player's running score.
 *
 * The level-clear check scans spawn points (not enemies) so it handles
 * deferred spawns correctly: an unfired deferred spawn point is NOT
 * satisfied, and the exit stays gated until that trigger fires and the
 * resulting enemy dies.
 */
UCLASS()
class QUAKE_API AQuakeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AQuakeGameMode();

	/** Denominator for the kill counter. Eligible spawn points at BeginPlay. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats")
	int32 KillsTotal = 0;

	/** Denominator for the secret counter. AQuakeTrigger_Secret actors at BeginPlay. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats")
	int32 SecretsTotal = 0;

	/**
	 * On-demand level-clear scan per SPEC 5.9. Iterates every
	 * AQuakeEnemySpawnPoint in the world and returns false if any eligible
	 * spawn point isn't satisfied (spawned + dead). The exit trigger gates
	 * on this when bGatedByClearCondition is set.
	 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	bool IsLevelCleared() const;

	/**
	 * Pure helper so unit tests can exercise the scan without a world.
	 * Returns false on any eligible-but-unsatisfied entry; true if every
	 * eligible spawn point is satisfied (including the empty-set case).
	 */
	static bool IsLevelClearedForSet(const TArray<const AQuakeEnemySpawnPoint*>& SpawnPoints);

	/**
	 * Current difficulty. Phase 12 wires this to UQuakeGameInstance so the
	 * value persists across OpenLevel / respawn; until then, Normal is the
	 * hardcoded default. Spawn points call this via IsEligible() to decide
	 * whether to participate in stat counting and the clear scan.
	 */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	EQuakeDifficulty GetDifficulty() const { return CurrentDifficulty; }

protected:
	virtual void BeginPlay() override;

	/** Phase 12 replaces this with a GameInstance read. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Difficulty")
	EQuakeDifficulty CurrentDifficulty = EQuakeDifficulty::Normal;
};
