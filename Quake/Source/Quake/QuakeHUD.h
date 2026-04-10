#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "QuakeHUD.generated.h"

/**
 * Minimal debug HUD for Phase 1. Draws the Quake speedometer — horizontal
 * speed, Z velocity, and the pawn's current MovementMode — in the top-left
 * corner. This is the diagnostic readout required by SPEC Phase 1 manual
 * verification ("speedometer should climb past 600 when strafe-jumping").
 *
 * Will be superseded in Phase 2 onward by the full AQuakeHUD + Slate overlay
 * (health, ammo, weapon icon, keys, etc.) per SPEC section 7.
 */
UCLASS()
class QUAKE_API AQuakeHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void DrawHUD() override;
};
