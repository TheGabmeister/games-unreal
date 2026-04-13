#pragma once

#include "CoreMinimal.h"
#include "QuakeTrigger.h"
#include "QuakeTrigger_Relay.generated.h"

/**
 * Phase 8 relay trigger per SPEC section 5.6. No type-specific fields —
 * it just fires the base class Targets on Activate. Used to chain buttons
 * → triggers → doors / secrets / exits, or for fan-out (one overlap fires
 * many targets).
 *
 * Authoring tip: disable TriggerVolume's collision on instances that are
 * pure relays (fired only by other triggers), leave it on for entry
 * triggers that also want to fire on pawn overlap.
 */
UCLASS()
class QUAKE_API AQuakeTrigger_Relay : public AQuakeTrigger
{
	GENERATED_BODY()
};
