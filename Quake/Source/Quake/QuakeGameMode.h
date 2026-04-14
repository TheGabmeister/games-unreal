#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "QuakeDifficulty.h"
#include "QuakeDifficultyMultipliers.h"
#include "QuakeGameMode.generated.h"

class AQuakeEnemySpawnPoint;
class UQuakeSaveGame;

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
	 * Current difficulty. Phase 12: authoritative storage lives on
	 * UQuakeGameInstance (survives OpenLevel and Character respawn). This
	 * getter forwards there; on null GameInstance (test harness) it falls
	 * back to Normal so pure helpers don't explode.
	 */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	EQuakeDifficulty GetDifficulty() const;

	/**
	 * DESIGN 6.1 multipliers table. Authored as a BP-tunable TMap; the C++
	 * constructor seeds the SPEC 6.1 defaults so an unconfigured BP still
	 * plays correctly. Read by AQuakeEnemyBase::ApplyDifficultyScaling and
	 * by the Nightmare pain-immunity gate in the AI controller.
	 */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	FQuakeDifficultyMultipliers GetDifficultyMultipliers() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Difficulty")
	TMap<EQuakeDifficulty, FQuakeDifficultyMultipliers> DifficultyTable;

	/**
	 * Pure helper behind GetDifficultyMultipliers so the per-difficulty
	 * lookup can be exercised by unit tests without a world. Returns the
	 * table entry if present, otherwise a Normal-default (1.0/1.0, no
	 * pain immunity) so callers never deal with null.
	 */
	static FQuakeDifficultyMultipliers LookupMultipliers(
		const TMap<EQuakeDifficulty, FQuakeDifficultyMultipliers>& Table,
		EQuakeDifficulty Difficulty);

	// --- Phase 13: failure + win flow ---

	/**
	 * If true, AQuakeTrigger_Exit shows the win screen instead of opening
	 * NextMapName. Set per-instance on `BP_QuakeGameMode_E1Mfinal` (or
	 * equivalent) in the final level of an episode. DESIGN 6.3.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Win")
	bool bIsFinalLevel = false;

	/** UMap to OpenLevel from the win screen / "Return to menu". */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Win")
	FName MainMenuMapName = NAME_None;

	/**
	 * DESIGN 6.4 restart sequence. Called by the PlayerController on
	 * fire-to-restart from the death screen. Restores GameInstance from
	 * the level-entry snapshot, clears per-life PlayerState, destroys the
	 * dead pawn, and respawns at PlayerStart.
	 */
	UFUNCTION(BlueprintCallable, Category = "Failure")
	void RequestRestartFromDeath(class AQuakePlayerController* PC);

	/** Pure helper: should an exit transition trigger the win screen flow? */
	static bool ShouldRouteToWinScreen(bool bIsFinal, FName NextMapName);

	// --- Phase 11: save/load orchestration ---

	/**
	 * Walk every IQuakeSaveable actor, populate Out.ActorRecords, capture
	 * the player's transform + PlayerState, and compute the consumed-pickup
	 * set from InitialPickupNames \ live pickup FNames. Called by
	 * UQuakeGameInstance::SaveCurrentState.
	 */
	void CaptureWorldSnapshot(UQuakeSaveGame& Out) const;

protected:
	virtual void BeginPlay() override;

	/**
	 * Post-BeginPlay orchestration for a pending load (DESIGN 6.2 steps 4-6):
	 * restore PlayerState, teleport the pawn, dispatch LoadState to
	 * IQuakeSaveable actors keyed by FName, destroy consumed pickups.
	 */
	void RestoreWorldFromSave(UQuakeSaveGame& Save);

	/**
	 * FName snapshot of every AQuakePickupBase live at BeginPlay. The save's
	 * ConsumedPickupNames is derived at save time as `Initial \ Live`.
	 */
	TArray<FName> InitialPickupNames;
};
