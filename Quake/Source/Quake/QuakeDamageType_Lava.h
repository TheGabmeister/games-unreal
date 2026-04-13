#pragma once

#include "CoreMinimal.h"
#include "QuakeDamageType.h"
#include "QuakeDamageType_Lava.generated.h"

/**
 * Lava damage. SPEC section 1.5 constructor overrides:
 *   - bSuppressesPain = true   -> burning in lava doesn't produce the
 *     damage-flash pain flinch every tick (the screen tint on its own
 *     communicates the hazard state).
 *   - bCausedByWorld  = true   -> UE built-in flag the attribution path
 *     treats as "no attacker" so no infighting grudge is seeded and the
 *     "killed by <instigator>" log reports <world> cleanly.
 */
UCLASS()
class QUAKE_API UQuakeDamageType_Lava : public UQuakeDamageType
{
	GENERATED_BODY()

public:
	UQuakeDamageType_Lava();
};
