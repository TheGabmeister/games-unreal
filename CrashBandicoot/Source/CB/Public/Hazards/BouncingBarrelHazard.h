#pragma once

#include "CoreMinimal.h"
#include "Hazards/HazardBase.h"
#include "BouncingBarrelHazard.generated.h"

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ABouncingBarrelHazard : public AHazardBase
{
	GENERATED_BODY()

public:
	ABouncingBarrelHazard();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float BounceHeight = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float BounceFrequency = 1.5f;

private:
	FVector GroundLocation;
	float TimeAccumulator = 0.0f;
};
