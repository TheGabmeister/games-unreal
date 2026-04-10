#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "QuakeGameInstance.generated.h"

/**
 * Persistent state owner that survives OpenLevel and Character respawn.
 *
 * Per SPEC section 1.4 and CLAUDE.md "Architecture: State Ownership", this
 * is the home for:
 *   - Inventory: weapons owned, ammo counts, armor
 *   - Level-entry snapshot (FQuakeInventorySnapshot) restored on death
 *   - Save-game reference
 *   - Player profile, difficulty
 *
 * Phase 0 scope: empty stub only. Fields are added in the phases that need
 * them — ammo/armor/weapons-owned in Phase 4 (Shotgun + Ammo + First Pickups),
 * difficulty in Phase 12, save/profile in Phase 11/13.
 *
 * Do NOT put keys, powerups, or per-level stats here — those live on
 * AQuakePlayerState (they reset on level transition; inventory does not).
 */
UCLASS()
class QUAKE_API UQuakeGameInstance : public UGameInstance
{
	GENERATED_BODY()
};
