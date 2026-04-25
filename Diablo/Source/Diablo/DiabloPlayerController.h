#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DiabloPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class ADiabloEnemy;
class ADroppedItem;
class UDiabloHUDWidget;
class UDiabloCharacterPanel;
class UDiabloInventoryPanel;

UCLASS(Abstract)
class DIABLO_API ADiabloPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADiabloPlayerController();

	void OnHeroDeath();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ClickAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CharPanelAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InventoryAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CastAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	TSubclassOf<UDiabloHUDWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	TSubclassOf<UDiabloCharacterPanel> CharPanelClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	TSubclassOf<UDiabloInventoryPanel> InventoryPanelClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackRange = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float PickupRange = 150.f;

private:
	void CreateHUD();
	void OnClickStarted();
	void OnCastStarted();
	void OnToggleCharPanel();
	void OnToggleInventory();
	void OnRespawnTimerExpired();

	UPROPERTY()
	TObjectPtr<ADiabloEnemy> TargetEnemy;

	UPROPERTY()
	TObjectPtr<ADroppedItem> TargetItem;

	UPROPERTY()
	TObjectPtr<UDiabloHUDWidget> HUDWidget;

	UPROPERTY()
	TObjectPtr<UDiabloCharacterPanel> CharPanel;

	UPROPERTY()
	TObjectPtr<UDiabloInventoryPanel> InventoryPanel;

	FTimerHandle RespawnTimerHandle;
};
