#pragma once

#include "CoreMinimal.h"
#include "QuakePowerup.generated.h"

/**
 * SPEC 4.3 powerup types. v1 ships Quad only; the rest are listed so the
 * enum doesn't churn when Phase 14+ polish adds them.
 */
UENUM(BlueprintType)
enum class EQuakePowerup : uint8
{
	None       UMETA(DisplayName = "None"),
	Quad       UMETA(DisplayName = "Quad Damage"),
	Pentagram  UMETA(DisplayName = "Pentagram of Protection"),
	Ring       UMETA(DisplayName = "Ring of Shadows"),
	Biosuit    UMETA(DisplayName = "Biosuit")
};
