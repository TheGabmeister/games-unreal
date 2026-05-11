#pragma once

#include "CoreMinimal.h"
#include "Projectiles/ProjectileBase.h"
#include "BeakerProjectile.generated.h"

class AGreenBlobEnemy;

UENUM(BlueprintType)
enum class EBeakerType : uint8 { Red, Green };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ABeakerProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:
	ABeakerProjectile();

protected:
	virtual void OnImpact(const FHitResult& Hit) override;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	EBeakerType BeakerType = EBeakerType::Red;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TSubclassOf<AGreenBlobEnemy> BlobClass;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float RedExplosionRadius = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float RedExplosionLingerDuration = 0.5f;
};
