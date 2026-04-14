#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "QuakeDifficulty.h"
#include "QuakeInventorySnapshot.h"
#include "QuakeKeyColor.h"
#include "QuakePlayerState.h"
#include "QuakeSaveable.h"
#include "QuakeSaveGame.generated.h"

/**
 * Phase 11 save payload per DESIGN 6.2.
 *
 * Three buckets of state:
 *   1. Profile (difficulty, name) — global-ish.
 *   2. GameInstance inventory snapshot — restored BEFORE OpenLevel.
 *   3. Per-level state — Character HP, player transform, PlayerState
 *      snapshot, per-actor records, consumed pickup FNames.
 *
 * Slots: `auto_<profile>` and `quick_<profile>`. v1 hardcodes the profile
 * to "default"; Phase 13 adds profile UI.
 */
UCLASS()
class QUAKE_API UQuakeSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// --- Profile ---

	UPROPERTY()
	FString ProfileName = TEXT("default");

	UPROPERTY()
	EQuakeDifficulty Difficulty = EQuakeDifficulty::Normal;

	// --- Inventory (from the player's UQuakeInventoryComponent) ---

	UPROPERTY()
	FQuakeInventorySnapshot InventorySnapshot;

	// --- Level ---

	UPROPERTY()
	FString CurrentLevelName;

	UPROPERTY()
	FTransform PlayerTransform;

	// --- Character ---

	UPROPERTY()
	float Health = 100.f;

	// --- PlayerState snapshot ---

	UPROPERTY()
	int32 Kills = 0;

	UPROPERTY()
	int32 Secrets = 0;

	UPROPERTY()
	int32 Deaths = 0;

	/** Seconds elapsed at save time. Translated to LevelStartTime on load. */
	UPROPERTY()
	float ElapsedAtSave = 0.f;

	UPROPERTY()
	TArray<FQuakeActivePowerup> ActivePowerups;

	UPROPERTY()
	TArray<EQuakeKeyColor> Keys;

	// --- Per-actor records ---

	UPROPERTY()
	TArray<FActorSaveRecord> ActorRecords;

	/**
	 * FNames of pickups that were present at level entry (InitialPickupNames
	 * snapshot) but no longer exist at save time. On load the GameMode
	 * destroys any re-spawned pickup whose FName is in this set. Avoids
	 * having to author "ghost records" for Destroy()-ed actors.
	 */
	UPROPERTY()
	TArray<FName> ConsumedPickupNames;
};
