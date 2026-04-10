#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "QuakePlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class QUAKE_API AQuakePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY()
	TObjectPtr<UInputAction> JumpAction;

protected:
	virtual void BeginPlay() override;

private:
	void SetupInputMappings();

	UPROPERTY()
	TObjectPtr<UInputMappingContext> InputMappingContext;
};
