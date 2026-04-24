#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DiabloPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class ADiabloEnemy;

UCLASS(Abstract)
class DIABLO_API ADiabloPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADiabloPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ClickAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackRange = 200.f;

private:
	void OnClickStarted();

	UPROPERTY()
	TObjectPtr<ADiabloEnemy> TargetEnemy;
};
