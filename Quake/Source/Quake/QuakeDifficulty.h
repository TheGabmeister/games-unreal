#pragma once

#include "CoreMinimal.h"
#include "QuakeDifficulty.generated.h"

/**
 * Difficulty tiers per SPEC section 6.1. Introduced in Phase 9 solely so
 * AQuakeEnemySpawnPoint can filter by MinDifficulty. The runtime wiring
 * (storage on UQuakeGameInstance, multiplier table on AQuakeGameMode, UI
 * selection) lands in Phase 12 — until then AQuakeGameMode::GetDifficulty
 * returns Normal as a sensible default.
 *
 * Values are ordered so `CurrentDifficulty >= MinDifficulty` is the
 * eligibility predicate: an "Easy" spawn point activates at every tier,
 * a "Nightmare" spawn point activates only at Nightmare.
 */
UENUM(BlueprintType)
enum class EQuakeDifficulty : uint8
{
	Easy      UMETA(DisplayName = "Easy"),
	Normal    UMETA(DisplayName = "Normal"),
	Hard      UMETA(DisplayName = "Hard"),
	Nightmare UMETA(DisplayName = "Nightmare"),
};
