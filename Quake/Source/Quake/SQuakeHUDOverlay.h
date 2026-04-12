#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class APlayerController;
class AQuakeCharacter;

/**
 * Phase 2 minimal HUD overlay. Pure Slate (no UMG, no BP) per SPEC
 * section 7. Polls the player pawn on paint via its owning controller —
 * the controller survives pawn replacement (death / respawn, future
 * teleport) whereas a pawn pointer would go stale the moment the body
 * dies. The controller is captured at construction; the character is
 * looked up via `PC->GetPawn<AQuakeCharacter>()` on every paint call.
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
		SLATE_ARGUMENT(TWeakObjectPtr<APlayerController>, OwningPlayerController)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TWeakObjectPtr<APlayerController> OwningPlayerController;

	/** Resolves the currently-possessed AQuakeCharacter, or nullptr. */
	const AQuakeCharacter* ResolvePlayerCharacter() const;

	FText GetHealthText() const;

	/** Phase 4: current weapon display name ("Axe" / "Shotgun" / ...). */
	FText GetWeaponText() const;

	/** Phase 4: ammo count for the currently-equipped weapon's AmmoType, or blank for Axe. */
	FText GetAmmoText() const;
};
