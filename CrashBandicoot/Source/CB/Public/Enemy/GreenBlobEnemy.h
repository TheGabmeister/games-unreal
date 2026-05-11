#pragma once

#include "CoreMinimal.h"
#include "Enemy/CBEnemyCharacterBase.h"
#include "GreenBlobEnemy.generated.h"

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AGreenBlobEnemy : public ACBEnemyCharacterBase
{
	GENERATED_BODY()

public:
	AGreenBlobEnemy();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float BounceInterval = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float BounceImpulseZ = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float BlobLifetime = 5.0f;

private:
	float BounceTimer = 0.0f;
};
