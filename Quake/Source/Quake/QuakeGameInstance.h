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
 * Phase 2 scope: armor fields (value + absorption ratio) are added here so
 * AQuakeCharacter::TakeDamage has somewhere persistent to read them from.
 * Default values (Armor=0, ArmorAbsorption=0) mean "no armor" — Phase 2
 * does not yet add armor pickups (those land in a later phase), so until
 * pickups exist these stay at zero and TakeDamage acts as if there is no
 * armor. The fields exist now because the unit test for the absorption
 * formula in SPEC Phase 2 needs the math wired up end-to-end.
 *
 * Weapons-owned and ammo counts are added in Phase 4. Difficulty in
 * Phase 12. Save/profile in Phase 11/13.
 *
 * Do NOT put keys, powerups, or per-level stats here — those live on
 * AQuakePlayerState (they reset on level transition; inventory does not).
 */
UCLASS()
class QUAKE_API UQuakeGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	/**
	 * Current armor points. Replaced (not stacked) on pickup of a higher
	 * tier per SPEC section 1.2. Caps at 200 (Red Armor max).
	 *
	 * Phase 2: stays at 0 since no armor pickups exist yet. Wired through
	 * the damage pipeline so the absorption math is exercised the moment
	 * pickups land in a later phase.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Armor")
	float Armor = 0.f;

	/**
	 * Current armor absorption ratio. Set on pickup to 0.3 (Green),
	 * 0.6 (Yellow), or 0.8 (Red) per SPEC section 1.2. Zero means no
	 * armor — TakeDamage skips the absorption step entirely when this is
	 * zero, even if Armor somehow has a value, so flipping armor "off"
	 * is just a matter of zeroing this field.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Armor")
	float ArmorAbsorption = 0.f;
};
