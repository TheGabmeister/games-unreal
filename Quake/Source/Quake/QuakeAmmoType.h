#pragma once

#include "CoreMinimal.h"
#include "QuakeAmmoType.generated.h"

/**
 * Ammo categories from SPEC section 2.1. One enum value per type; the
 * integer values are stable (0..3) so they can key a TMap on
 * UQuakeGameInstance. None is a sentinel for weapons that consume no ammo
 * (the Axe).
 *
 * Carry caps and pickup amounts live on UQuakeGameInstance::GetAmmoCap and
 * as UPROPERTY defaults on the ammo pickup actors respectively — NOT on
 * this enum — so the enum stays a pure tag.
 */
UENUM(BlueprintType)
enum class EQuakeAmmoType : uint8
{
	None    UMETA(DisplayName = "None"),
	Shells  UMETA(DisplayName = "Shells"),
	Nails   UMETA(DisplayName = "Nails"),
	Rockets UMETA(DisplayName = "Rockets"),
	Cells   UMETA(DisplayName = "Cells"),
};
