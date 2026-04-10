#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Templates/SharedPointer.h"
#include "QuakeHUD.generated.h"

class SQuakeHUDOverlay;

/**
 * AQuakeHUD owns the player-facing HUD. It draws two pieces:
 *
 *   1. The Phase 1 dev speedometer (horizontal speed / Z velocity / movement
 *      mode), via DrawHUD on the Canvas. This is the diagnostic readout the
 *      strafe-jump feel test relies on (SPEC Phase 1 manual verification);
 *      it stays in DrawHUD because Slate isn't the right tool for a debug
 *      overlay we expect to remove later.
 *
 *   2. The Phase 2 Slate health overlay (SQuakeHUDOverlay), constructed in
 *      BeginPlay and added to the viewport via the GameViewport. Polls
 *      AQuakeCharacter on paint via a weak pointer.
 *
 * In later phases the Slate widget grows ammo / armor / weapon bar / keys /
 * powerups per SPEC section 7. The DrawHUD speedometer can then be deleted
 * once strafe-jump tuning is final.
 */
UCLASS()
class QUAKE_API AQuakeHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void DrawHUD() override;

private:
	TSharedPtr<SQuakeHUDOverlay> OverlayWidget;
};
