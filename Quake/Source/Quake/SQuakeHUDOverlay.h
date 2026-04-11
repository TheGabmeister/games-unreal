#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class AQuakeCharacter;

/**
 * Phase 2 minimal HUD overlay. Pure Slate (no UMG, no BP) per SPEC
 * section 7. Polls AQuakeCharacter on paint via a weak pointer captured
 * at construction.
 *
 * Phase 2 displays only Health — the rest of the table in SPEC section 7
 * (armor, ammo, weapon bar, keys, powerups, crosshair) is added in later
 * phases. Layout is bottom-left text per the SPEC HUD positions table.
 *
 * The widget is constructed and added to the viewport by AQuakeHUD in
 * BeginPlay; AQuakeHUD also removes it on EndPlay so the lifetime tracks
 * the player.
 */
class QUAKE_API SQuakeHUDOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuakeHUDOverlay) {}
		SLATE_ARGUMENT(TWeakObjectPtr<AQuakeCharacter>, PlayerCharacter)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TWeakObjectPtr<AQuakeCharacter> PlayerCharacter;

	FText GetHealthText() const;

	/** Phase 4: current weapon display name ("Axe" / "Shotgun" / ...). */
	FText GetWeaponText() const;

	/** Phase 4: ammo count for the currently-equipped weapon's AmmoType, or blank for Axe. */
	FText GetAmmoText() const;
};
