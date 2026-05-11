#pragma once

#include "CoreMinimal.h"
#include "Hazards/HazardBase.h"
#include "FlyingFishHazard.generated.h"

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AFlyingFishHazard : public AHazardBase
{
	GENERATED_BODY()

public:
	AFlyingFishHazard();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float JumpHeight = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float JumpDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float RestDuration = 2.0f;

private:
	FVector RestLocation;
	float Timer = 0.0f;
	bool bJumping = false;
};
