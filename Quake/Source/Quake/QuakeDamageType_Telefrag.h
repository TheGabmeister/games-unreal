#pragma once

#include "CoreMinimal.h"
#include "QuakeDamageType.h"
#include "QuakeDamageType_Telefrag.generated.h"

/**
 * Telefrag / kill-volume damage. Used by AQuakeTrigger_Hurt (SPEC 5.6) and
 * later by the telefrag rule when a teleport destination is occupied.
 *
 * bSuppressesPain = true so a kill volume doesn't produce a flinch animation
 * the frame before it kills the target. The damage amount (typically 10000
 * for instakills) is passed by the caller, not stored here — damage types
 * are stateless tags.
 */
UCLASS()
class QUAKE_API UQuakeDamageType_Telefrag : public UQuakeDamageType
{
	GENERATED_BODY()

public:
	UQuakeDamageType_Telefrag();
};
