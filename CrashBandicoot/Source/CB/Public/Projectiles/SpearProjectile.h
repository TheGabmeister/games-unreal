#pragma once

#include "CoreMinimal.h"
#include "Projectiles/ProjectileBase.h"
#include "SpearProjectile.generated.h"

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ASpearProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:
	ASpearProjectile();
};
