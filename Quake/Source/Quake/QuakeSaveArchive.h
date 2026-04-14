#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"

/**
 * Phase 11 serialization helpers per DESIGN 6.2.
 *
 * Wraps FMemoryWriter / FMemoryReader + FObjectAndNameAsStringProxyArchive
 * so every IQuakeSaveable::SaveState body is one call. Any UPROPERTY
 * tagged with `meta = (SaveGame)` round-trips; everything else is skipped
 * because ArIsSaveGame = true sets the CPF_SaveGame property-flag filter.
 *
 * Class-reference UPROPERTYs (TSubclassOf, TSoftClassPtr) are written as
 * path strings via the proxy archive and re-resolved on load with
 * bLoadIfFindFails = true. Safe as long as BP class asset paths are not
 * renamed between saves.
 */
namespace QuakeSaveArchive
{
	/** Serialize UObject's SaveGame-marked properties into OutBytes. */
	QUAKE_API void WriteSaveProperties(UObject* Object, TArray<uint8>& OutBytes);

	/** Deserialize SaveGame-marked properties from InBytes into Object. */
	QUAKE_API void ReadSaveProperties(UObject* Object, const TArray<uint8>& InBytes);

	/**
	 * Pure set-difference: returns names in Initial that are not in Live.
	 * Exposed as a free function so the consumed-pickup math is unit-testable
	 * without a world. Used by AQuakeGameMode at save time.
	 */
	QUAKE_API TArray<FName> ComputeConsumedNames(
		const TArray<FName>& InitialNames,
		const TArray<FName>& LiveNames);

	/**
	 * DESIGN 6.2 "No mid-air saves" predicate. F5 is accepted iff the
	 * player is grounded, not flinching, and not dead. Pure function so
	 * the matrix is unit-testable without a world.
	 */
	QUAKE_API bool CanQuickSave(
		EMovementMode MovementMode,
		bool bIsInPain,
		bool bIsDead);

	/**
	 * Time-base translation for PlayerState::LevelStartTime on load.
	 * GetTimeElapsed() = WorldTimeNow - LevelStartTime; to restore a
	 * saved ElapsedAtSave we rewind LevelStartTime by that amount.
	 */
	QUAKE_API double ComputeRestoredLevelStartTime(
		double WorldTimeNow,
		float ElapsedAtSave);
}
