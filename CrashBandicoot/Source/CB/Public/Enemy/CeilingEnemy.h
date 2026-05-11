#pragma once

#include "CoreMinimal.h"
#include "Enemy/CBEnemyCharacterBase.h"
#include "CeilingEnemy.generated.h"

UENUM(BlueprintType)
enum class ECeilingEnemyState : uint8 { Hanging, Dropping, Landed, Climbing, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyCeiling : public ACBEnemyCharacterBase
{
	GENERATED_BODY()

public:
	AEnemyCeiling();
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "CB")
	ECeilingEnemyState CurrentState = ECeilingEnemyState::Hanging;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float DropSpeed = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float ClimbSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float LandedPauseDuration = 1.0f;

	FVector CeilingLocation;
	FVector GroundLocation;
	float StateTimer = 0.0f;
	bool bTriggered = false;

	void SetState(ECeilingEnemyState NewState);
	virtual void HandleDefeat() override;
	virtual void BeginPlay() override;
};
