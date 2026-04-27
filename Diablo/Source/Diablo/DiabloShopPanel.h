#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiabloShopPanel.generated.h"

class UTextBlock;
class UButton;
class UVerticalBox;
class UBorder;
class ADiabloPlayerController;
class ADiabloNPC;
class UInventoryComponent;

UCLASS(Abstract)
class DIABLO_API UDiabloShopPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(ADiabloPlayerController* InController, ADiabloNPC* InNPC);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeDestruct() override;

private:
	void RefreshShop();
	void BuyItem(int32 StockIndex);
	void SellItem(int32 SellIndex);
	void OnInventoryChanged();

	UFUNCTION()
	void OnCloseClicked();

	UFUNCTION()
	void OnRepairClicked();

	UPROPERTY()
	TObjectPtr<ADiabloPlayerController> OwnerController;

	UPROPERTY()
	TObjectPtr<ADiabloNPC> ShopNPC;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> PlayerInventory;

	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY()
	TObjectPtr<UTextBlock> GoldText;

	UPROPERTY()
	TObjectPtr<UVerticalBox> BuyListBox;

	UPROPERTY()
	TObjectPtr<UVerticalBox> SellListBox;

	UPROPERTY()
	TObjectPtr<UButton> CloseButton;

	UPROPERTY()
	TObjectPtr<UButton> RepairButton;

	UPROPERTY()
	TObjectPtr<UTextBlock> RepairCostText;

	UPROPERTY()
	TArray<TObjectPtr<UBorder>> BuyRows;

	UPROPERTY()
	TArray<TObjectPtr<UBorder>> SellRows;

	struct FSellEntry { int32 GridX; int32 GridY; };
	TArray<FSellEntry> SellEntries;

	FDelegateHandle InventoryChangedHandle;
};
