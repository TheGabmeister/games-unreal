#pragma once

#include "CoreMinimal.h"
#include "QuakeAmmoType.h"
#include "QuakeInventorySnapshot.generated.h"

class AQuakeWeaponBase;

/**
 * Phase 13 level-entry inventory snapshot per DESIGN 6.4 step 3.
 *
 * Captured on AQuakeGameMode::BeginPlay (after any save-load restore) so
 * the player's inventory at level entry is recoverable on death-restart.
 * Stored on UQuakeGameInstance because the snapshot must outlive the
 * dying pawn but not OpenLevel — a fresh level captures a fresh snapshot.
 *
 * Health is intentionally NOT in the snapshot. DESIGN 6.4 step 5: "new
 * pawn BeginPlay sets HP to 100" — death always restores full health.
 */
USTRUCT()
struct FQuakeInventorySnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	float Armor = 0.f;

	UPROPERTY()
	float ArmorAbsorption = 0.f;

	UPROPERTY()
	TArray<TSubclassOf<AQuakeWeaponBase>> OwnedWeaponClasses;

	UPROPERTY()
	TMap<EQuakeAmmoType, int32> AmmoCounts;

	/** Set true once SnapshotForLevelEntry runs. Empty snapshots are no-ops on restore. */
	UPROPERTY()
	bool bValid = false;
};
