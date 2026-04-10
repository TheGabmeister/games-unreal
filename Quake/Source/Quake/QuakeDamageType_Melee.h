#pragma once

#include "CoreMinimal.h"
#include "QuakeDamageType.h"
#include "QuakeDamageType_Melee.generated.h"

/**
 * Melee damage type — used by the Axe (Phase 2) and any future melee enemy
 * attacks (Knight charge, Fiend leap, Shambler claw).
 *
 * Per SPEC section 1.5, leaf subclasses add NO new properties — they only
 * override defaults in their constructor. Melee uses every base default
 * unchanged (no armor bypass, normal pain, normal self-damage, normal
 * knockback) so the constructor body is intentionally empty. The class
 * exists purely as a tag so call sites can pass
 * UQuakeDamageType_Melee::StaticClass() into ApplyPointDamage and so
 * downstream code can attribute the hit to a melee weapon if it ever needs
 * to differentiate (e.g., a future "axe-only achievement" doesn't need any
 * branching in TakeDamage — it scans the damage type class instead).
 */
UCLASS()
class QUAKE_API UQuakeDamageType_Melee : public UQuakeDamageType
{
	GENERATED_BODY()

public:
	UQuakeDamageType_Melee();
};
