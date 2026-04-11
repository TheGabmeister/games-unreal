#pragma once

#include "CoreMinimal.h"
#include "QuakeDamageType.h"
#include "QuakeDamageType_Bullet.generated.h"

/**
 * Bullet damage type — used by hitscan rifles (Phase 3 Grunt) and any other
 * non-shotgun hitscan attack. Added in Phase 3.
 *
 * Per SPEC section 1.5, leaf subclasses add NO new properties — they only
 * override defaults in their constructor. Bullet uses every base default
 * unchanged (no armor bypass, normal pain, normal self-damage, normal
 * knockback) so the constructor body is intentionally empty. The class
 * exists purely as a tag so call sites pass
 * UQuakeDamageType_Bullet::StaticClass() into ApplyPointDamage and so future
 * code can attribute the hit source without branching on leaf class identity.
 */
UCLASS()
class QUAKE_API UQuakeDamageType_Bullet : public UQuakeDamageType
{
	GENERATED_BODY()

public:
	UQuakeDamageType_Bullet();
};
