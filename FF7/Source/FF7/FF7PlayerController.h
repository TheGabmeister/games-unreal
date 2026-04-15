// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FF7PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * Field / battle player controller (SPEC §2.3).
 * EnhancedInput assets (IMC + IAs) are assigned on the BP subclass so
 * designers can swap them without touching C++. On possession, the IMC is
 * pushed to the local player's input subsystem; bindings fire C++ handlers.
 */
UCLASS()
class FF7_API AFF7PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FF7|Input")
	TSoftObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FF7|Input")
	TSoftObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FF7|Input")
	TSoftObjectPtr<UInputAction> IA_Interact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FF7|Input")
	TSoftObjectPtr<UInputAction> IA_MenuToggle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FF7|Input")
	TSoftObjectPtr<UInputAction> IA_Escape;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void SetupInputComponent() override;

private:
	void HandleMove(const FInputActionValue& Value);
};
