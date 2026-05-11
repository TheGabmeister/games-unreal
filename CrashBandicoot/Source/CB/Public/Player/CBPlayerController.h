#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CBPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UCBGameplayHUD;

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ACBPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACBPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void SetupInputComponent() override;

	// --- Input Actions ---

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<UInputAction> IA_MoveAxis;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<UInputAction> IA_Spin;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<UInputAction> IA_PauseMenu;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<UInputMappingContext> IMC_Gameplay;

	// --- HUD ---

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TSubclassOf<UCBGameplayHUD> GameplayHUDClass;

	UPROPERTY(BlueprintReadOnly, Category = "CB")
	TObjectPtr<UCBGameplayHUD> GameplayHUD;

private:
	void Input_Move(const struct FInputActionValue& Value);
	void Input_Jump(const struct FInputActionValue& Value);
	void Input_StopJump(const struct FInputActionValue& Value);
	void Input_Spin(const struct FInputActionValue& Value);
	void Input_PauseMenu(const struct FInputActionValue& Value);

	void BindToGameInstance();

	UFUNCTION()
	void OnLivesChanged(int32 NewLives);

	UFUNCTION()
	void OnWumpaChanged(int32 NewCount);
};
