#pragma once

#include "CoreMinimal.h"
#include "QuakeKeyColor.generated.h"

/**
 * SPEC 4.4 key colors. Extracted to its own header (rather than living on
 * AQuakeDoor) because Phase 10 has three consumers outside the door — key
 * pickups, PlayerState storage, and the HUD indicator — and pulling
 * QuakeDoor.h into each of those to reach an enum is noise.
 */
UENUM(BlueprintType)
enum class EQuakeKeyColor : uint8
{
	None   UMETA(DisplayName = "None"),
	Silver UMETA(DisplayName = "Silver"),
	Gold   UMETA(DisplayName = "Gold")
};
