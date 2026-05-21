#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TH2PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class TONYHAWK2_API ATH2PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> SkatingMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> OllieAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> BrakeAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SwitchStanceAction;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void HandleMove(const struct FInputActionValue& Value);
	void HandleOllieStarted(const struct FInputActionValue& Value);
	void HandleOllieCompleted(const struct FInputActionValue& Value);
	void HandleBrakeStarted(const struct FInputActionValue& Value);
	void HandleBrakeCompleted(const struct FInputActionValue& Value);
	void HandleSwitchStance(const struct FInputActionValue& Value);
};
