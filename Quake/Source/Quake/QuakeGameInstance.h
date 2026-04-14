#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "QuakeAmmoType.h"
#include "QuakeDifficulty.h"
#include "QuakeInventorySnapshot.h"
#include "QuakeGameInstance.generated.h"

class AQuakeWeaponBase;
class UDataTable;
class UQuakeInventoryComponent;
class UQuakeSaveGame;

/**
 * Persistent object that survives OpenLevel and Character respawn.
 *
 * Role after the inventory-component refactor: passive mailbox + save
 * plumbing + difficulty + sound-table reference. Live inventory (ammo,
 * armor, weapon classes) lives on UQuakeInventoryComponent attached to
 * AQuakeCharacter; the pawn dies on OpenLevel, but this GameInstance
 * survives and holds `TransitSnapshot` as a one-shot handoff that the
 * next pawn's component consumes in InitializeComponent. Matches the
 * Quake 1 `parm1..parm16` globals pattern and keeps MP migration small
 * (the component replicates on the pawn; this object stays local).
 *
 * Do NOT put gameplay state here. Do NOT put keys, powerups, or per-level
 * stats here either — those live on AQuakePlayerState.
 */
UCLASS()
class QUAKE_API UQuakeGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UQuakeGameInstance();

	// --- Cross-level inventory handoff ---

	/**
	 * Passive mailbox consumed by UQuakeInventoryComponent::InitializeComponent
	 * on the next-spawned pawn. Populated by:
	 *   - AQuakeTrigger_Exit before OpenLevel (level transition)
	 *   - LoadFromSlot from save-game (quickload)
	 *   - RestoreFromLevelEntrySnapshot (death-restart, copies LevelEntrySnapshot)
	 * Cleared (`bValid = false`) after the component reads it.
	 */
	UPROPERTY()
	FQuakeInventorySnapshot TransitSnapshot;

	// --- Phase 11: save/load ---

	/**
	 * Snapshot the current game state and write it to SlotName. Captures
	 * GameInstance inventory + the current level's PlayerState / per-actor
	 * state via the authoritative GameMode (DESIGN 6.2). Returns true on
	 * disk write success.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveCurrentState(const FString& SlotName);

	/**
	 * Load from SlotName: restore GameInstance fields immediately, stash
	 * the save as a pending load, then OpenLevel on the saved level name.
	 * The new GameMode's BeginPlay pulls the pending load and finishes
	 * the restore (pawn transform, PlayerState, per-actor records,
	 * consumed pickups). Returns true on slot load success.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadFromSlot(const FString& SlotName);

	/**
	 * GameMode pulls this on BeginPlay. Returns the pending load (if any)
	 * and clears it. Null when the world was opened via normal navigation
	 * (not via LoadFromSlot).
	 */
	UQuakeSaveGame* ConsumePendingLoad();

	/** Format "auto_<profile>" / "quick_<profile>". v1: profile = "default". */
	static FString BuildAutoSlotName();
	static FString BuildQuickSlotName();

	// --- Phase 13: level-entry inventory snapshot (DESIGN 6.4 step 3) ---

	/**
	 * Capture the live component's state into LevelEntrySnapshot. Called by
	 * AQuakeGameMode::BeginPlay after any save-load restore so the snapshot
	 * reflects "what the player walked in with this level." The argument is
	 * the player pawn's inventory component (lookup lives on the caller so
	 * this method stays testable without a world).
	 */
	void SnapshotForLevelEntry(const UQuakeInventoryComponent* Comp);

	/**
	 * Queue LevelEntrySnapshot into TransitSnapshot so the next-spawned
	 * pawn's component consumes it on InitializeComponent. Called by the
	 * death-restart flow BEFORE the new pawn spawns. No-op on an invalid
	 * snapshot (fresh-game death before the first level-entry capture —
	 * component falls back to UPROPERTY defaults, same as new-game start).
	 */
	void RestoreFromLevelEntrySnapshot();

	const FQuakeInventorySnapshot& GetLevelEntrySnapshot() const { return LevelEntrySnapshot; }

	// --- Phase 12: difficulty ---

	/**
	 * Current difficulty. Persists across OpenLevel and Character respawn
	 * per DESIGN 6.1 "cannot change mid-playthrough". The GameMode reads
	 * this on BeginPlay and feeds it to AQuakeEnemyBase::ApplyDifficultyScaling
	 * and AQuakeEnemySpawnPoint::IsEligible.
	 */
	UFUNCTION(BlueprintCallable, Category = "Difficulty")
	EQuakeDifficulty GetDifficulty() const { return CurrentDifficulty; }

	UFUNCTION(BlueprintCallable, Category = "Difficulty")
	void SetDifficulty(EQuakeDifficulty NewDifficulty) { CurrentDifficulty = NewDifficulty; }

	// --- Phase 14: audio (DESIGN 8.1) ---

	/**
	 * DataTable of FQuakeSoundEvent rows. Lives here (not on the subsystem)
	 * because UGameInstanceSubsystem cannot be Blueprint-subclassed and we
	 * need a BP slot for asset assignment. UQuakeSoundManager pulls this
	 * via GetGameInstance() on first PlaySound and caches the resolved table.
	 *
	 * Soft pointer so the cooked package doesn't pull the table (and every
	 * referenced sound asset) into the GameInstance's CDO.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<UDataTable> SoundEventTable;

	// --- Init hook ---

	virtual void Init() override;

	/**
	 * Resolve the UQuakeGameInstance off any WorldContext and assert it
	 * exists as the right subclass. Replaces the "null-tolerant facade"
	 * pattern that silently degraded when GameInstanceClass was
	 * misconfigured in DefaultEngine.ini — a wrong-class config is an
	 * authoring error that should crash immediately, not produce
	 * empty-inventory gameplay. Returns a non-null pointer or hits checkf.
	 *
	 * Safe to call from any game-thread site that has a valid WorldContext.
	 * Display / polling paths (HUD, menu) that legitimately run outside a
	 * configured game (e.g. loading screens) should keep using the
	 * null-tolerant GetGameInstance<>() directly.
	 */
	static UQuakeGameInstance* GetChecked(const UObject* WorldContext);

private:
	/** DESIGN 6.1 difficulty state. Defaults to Normal; set via SetDifficulty. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Difficulty",
		meta = (AllowPrivateAccess = "true"))
	EQuakeDifficulty CurrentDifficulty = EQuakeDifficulty::Normal;

	/** DESIGN 6.4 step 3: captured at level entry, restored on death. */
	UPROPERTY()
	FQuakeInventorySnapshot LevelEntrySnapshot;

	/**
	 * Populated by LoadFromSlot before OpenLevel; consumed by the new
	 * world's AQuakeGameMode::BeginPlay. Kept as a UPROPERTY so the GC
	 * keeps the SaveGame alive across the level swap.
	 */
	UPROPERTY()
	TObjectPtr<UQuakeSaveGame> PendingLoad;

	/** Write inventory + profile fields onto a SaveGame we own. */
	void CaptureInventorySnapshot(UQuakeSaveGame& Out) const;

	/** Restore inventory + profile fields from a loaded SaveGame. */
	void ApplyInventorySnapshot(const UQuakeSaveGame& In);

	friend class AQuakeGameMode;
};
