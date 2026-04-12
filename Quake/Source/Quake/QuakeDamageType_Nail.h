#pragma once

#include "CoreMinimal.h"
#include "QuakeDamageType.h"
#include "QuakeDamageType_Nail.generated.h"

/**
 * Nail damage type — used by the Nailgun (Phase 6) and the Super Nailgun.
 * Per SPEC section 1.5 leaf subclasses add NO new properties and only
 * override defaults in their constructor. Nail uses every base default
 * unchanged (normal armor absorption, normal pain, normal self-damage,
 * normal knockback) so the constructor body is intentionally empty. The
 * class exists purely as a tag so call sites pass
 * UQuakeDamageType_Nail::StaticClass() into ApplyPointDamage and so future
 * code can attribute a hit to "nail source" without branching on leaf class
 * identity.
 */
UCLASS()
class QUAKE_API UQuakeDamageType_Nail : public UQuakeDamageType
{
	GENERATED_BODY()

public:
	UQuakeDamageType_Nail();
};
