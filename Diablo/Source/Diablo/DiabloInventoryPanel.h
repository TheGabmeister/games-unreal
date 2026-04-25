#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemDefinition.h"
#include "DiabloInventoryPanel.generated.h"

class UInventoryComponent;
class UCanvasPanel;
class UBorder;
class UImage;
class UTextBlock;

UCLASS(Abstract)
class DIABLO_API UDiabloInventoryPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitForInventory(UInventoryComponent* InInventory);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

private:
	void OnInventoryChanged();
	void RefreshGrid();
	void RefreshEquipment();
	void RefreshGold();

	bool HitTestGrid(const FGeometry& InGeometry, const FVector2D& ScreenPos, int32& OutX, int32& OutY) const;
	bool HitTestEquip(const FGeometry& InGeometry, const FVector2D& ScreenPos, EEquipSlot& OutSlot) const;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> CachedInventory;

	UPROPERTY()
	TObjectPtr<UCanvasPanel> GridCanvas;

	UPROPERTY()
	TObjectPtr<UTextBlock> GoldText;

	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY()
	TObjectPtr<UTextBlock> HoverText;

	UPROPERTY()
	TArray<TObjectPtr<UBorder>> GridCellWidgets;

	UPROPERTY()
	TMap<EEquipSlot, TObjectPtr<UBorder>> EquipSlotWidgets;

	FDelegateHandle InventoryChangedHandle;

	static constexpr float CellSize = 36.f;
	static constexpr float CellPadding = 1.f;

	int32 DragSourceX = -1;
	int32 DragSourceY = -1;
	EEquipSlot DragSourceSlot = EEquipSlot::None;
	bool bDragFromEquip = false;
};
