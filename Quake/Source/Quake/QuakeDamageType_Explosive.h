#pragma once

#include "CoreMinimal.h"
#include "QuakeDamageType.h"
#include "QuakeDamageType_Explosive.generated.h"

/**
 * Explosive damage type — used by the Rocket Launcher (Phase 5) and later by
 * the Grenade Launcher (Phase 7). SPEC section 1.5 constructor overrides:
 *
 *   - SelfDamageScale = 0.5  -> rocket jumps are survivable (player takes
 *     half the splash they inflict on themselves).
 *   - KnockbackScale  = 4.0  -> splash knockback multiplier vs. the base
 *     formula in AQuakeCharacter::TakeDamage (ScaledDamage * 30 * Scale).
 *     Large multiplier is what makes rocket-jumps launch the player.
 *
 * Per SPEC section 1.5, the splash RADIUS lives on the weapon (the Rocket
 * Launcher), not here — a damage type is a stateless tag for metadata, not
 * a bag of weapon settings.
 */
UCLASS()
class QUAKE_API UQuakeDamageType_Explosive : public UQuakeDamageType
{
	GENERATED_BODY()

public:
	UQuakeDamageType_Explosive();
};
