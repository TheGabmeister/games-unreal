#include "DiabloInventoryPanel.h"
#include "InventoryComponent.h"
#include "InventoryDragDrop.h"
#include "ItemInstance.h"
#include "Diablo.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void UDiabloInventoryPanel::InitForInventory(UInventoryComponent* InInventory)
{
	if (CachedInventory && InventoryChangedHandle.IsValid())
	{
		CachedInventory->OnInventoryChanged.Remove(InventoryChangedHandle);
		InventoryChangedHandle.Reset();
	}

	CachedInventory = InInventory;

	if (CachedInventory)
	{
		InventoryChangedHandle = CachedInventory->OnInventoryChanged.AddUObject(
			this, &UDiabloInventoryPanel::OnInventoryChanged);
		RefreshGrid();
		RefreshEquipment();
		RefreshGold();
	}
}

void UDiabloInventoryPanel::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshGrid();
	RefreshEquipment();
	RefreshGold();
}

void UDiabloInventoryPanel::NativeDestruct()
{
	if (CachedInventory && InventoryChangedHandle.IsValid())
	{
		CachedInventory->OnInventoryChanged.Remove(InventoryChangedHandle);
		InventoryChangedHandle.Reset();
	}
	Super::NativeDestruct();
}

void UDiabloInventoryPanel::OnInventoryChanged()
{
	RefreshGrid();
	RefreshEquipment();
	RefreshGold();
}

// ---------------------------------------------------------------------------
// Input: mouse down, drag, drop
// ---------------------------------------------------------------------------

static bool IsScreenPosInsideWidget(const UWidget* Widget, const FVector2D& ScreenPos)
{
	if (!Widget) return false;
	const TSharedPtr<SWidget> SlateWidget = Widget->GetCachedWidget();
	if (!SlateWidget) return false;

	const FGeometry Geom = SlateWidget->GetTickSpaceGeometry();
	const FVector2D LocalPos = Geom.AbsoluteToLocal(ScreenPos);
	const FVector2D Size = Geom.GetLocalSize();
	return LocalPos.X >= 0.f && LocalPos.Y >= 0.f && LocalPos.X <= Size.X && LocalPos.Y <= Size.Y;
}

bool UDiabloInventoryPanel::HitTestGrid(const FGeometry& InGeometry, const FVector2D& ScreenPos,
	int32& OutX, int32& OutY) const
{
	for (int32 y = 0; y < UInventoryComponent::GridHeight; ++y)
	{
		for (int32 x = 0; x < UInventoryComponent::GridWidth; ++x)
		{
			const int32 Idx = y * UInventoryComponent::GridWidth + x;
			if (Idx >= GridCellWidgets.Num()) continue;

			if (IsScreenPosInsideWidget(GridCellWidgets[Idx], ScreenPos))
			{
				OutX = x;
				OutY = y;
				return true;
			}
		}
	}
	return false;
}

bool UDiabloInventoryPanel::HitTestEquip(const FGeometry& InGeometry, const FVector2D& ScreenPos,
	EEquipSlot& OutSlot) const
{
	for (const auto& Pair : EquipSlotWidgets)
	{
		if (IsScreenPosInsideWidget(Pair.Value, ScreenPos))
		{
			OutSlot = Pair.Key;
			return true;
		}
	}
	return false;
}

FReply UDiabloInventoryPanel::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!CachedInventory) return FReply::Unhandled();

	const FVector2D ScreenPos = InMouseEvent.GetScreenSpacePosition();

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		int32 GridX, GridY;
		if (HitTestGrid(InGeometry, ScreenPos, GridX, GridY))
		{
			CachedInventory->UseItem(GridX, GridY);
			return FReply::Handled();
		}

		EEquipSlot HitSlot;
		if (HitTestEquip(InGeometry, ScreenPos, HitSlot))
		{
			CachedInventory->Unequip(HitSlot);
			return FReply::Handled();
		}
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		int32 GridX, GridY;
		EEquipSlot HitSlot;

		if (HitTestGrid(InGeometry, ScreenPos, GridX, GridY) &&
			CachedInventory->GetItemAt(GridX, GridY))
		{
			DragSourceX = GridX;
			DragSourceY = GridY;
			bDragFromEquip = false;
			return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		}

		if (HitTestEquip(InGeometry, ScreenPos, HitSlot) &&
			CachedInventory->HasEquipped(HitSlot))
		{
			DragSourceSlot = HitSlot;
			bDragFromEquip = true;
			return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		}
	}

	return FReply::Unhandled();
}

void UDiabloInventoryPanel::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	UInventoryDragDrop* DragOp = NewObject<UInventoryDragDrop>();
	DragOp->Pivot = EDragPivot::CenterCenter;

	if (bDragFromEquip)
	{
		DragOp->Source = EInventoryDragSource::Equipment;
		DragOp->SourceEquipSlot = DragSourceSlot;
	}
	else
	{
		DragOp->Source = EInventoryDragSource::Grid;
		DragOp->SourceGridX = DragSourceX;
		DragOp->SourceGridY = DragSourceY;
	}

	UBorder* Visual = NewObject<UBorder>(this);
	Visual->SetBrushColor(FLinearColor(0.8f, 0.7f, 0.3f, 0.7f));

	DragOp->DefaultDragVisual = Visual;
	OutOperation = DragOp;
}

bool UDiabloInventoryPanel::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UInventoryDragDrop* DragOp = Cast<UInventoryDragDrop>(InOperation);
	if (!DragOp || !CachedInventory) return false;

	const FVector2D ScreenPos = InDragDropEvent.GetScreenSpacePosition();
	int32 DropX, DropY;
	EEquipSlot DropSlot;

	if (DragOp->Source == EInventoryDragSource::Grid)
	{
		if (HitTestGrid(InGeometry, ScreenPos, DropX, DropY))
		{
			return CachedInventory->MoveItem(DragOp->SourceGridX, DragOp->SourceGridY, DropX, DropY);
		}
		if (HitTestEquip(InGeometry, ScreenPos, DropSlot))
		{
			return CachedInventory->Equip(DragOp->SourceGridX, DragOp->SourceGridY);
		}
	}

	if (DragOp->Source == EInventoryDragSource::Equipment)
	{
		if (HitTestGrid(InGeometry, ScreenPos, DropX, DropY))
		{
			return CachedInventory->Unequip(DragOp->SourceEquipSlot);
		}
	}

	return false;
}

FReply UDiabloInventoryPanel::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!CachedInventory || !HoverText) return FReply::Unhandled();

	const FVector2D ScreenPos = InMouseEvent.GetScreenSpacePosition();
	FString ItemName;

	int32 GridX, GridY;
	if (HitTestGrid(InGeometry, ScreenPos, GridX, GridY))
	{
		if (const FItemInstance* Item = CachedInventory->GetItemAt(GridX, GridY))
		{
			ItemName = Item->Definition->DisplayName.ToString();
		}
	}

	EEquipSlot HitSlot;
	if (ItemName.IsEmpty() && HitTestEquip(InGeometry, ScreenPos, HitSlot))
	{
		const FItemInstance& Equipped = CachedInventory->GetEquipped(HitSlot);
		if (Equipped.IsValid())
		{
			ItemName = Equipped.Definition->DisplayName.ToString();
		}
	}

	HoverText->SetText(FText::FromString(ItemName));

	return FReply::Unhandled();
}

void UDiabloInventoryPanel::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (HoverText)
	{
		HoverText->SetText(FText::GetEmpty());
	}
}

// ---------------------------------------------------------------------------
// Refresh
// ---------------------------------------------------------------------------

void UDiabloInventoryPanel::RefreshGrid()
{
	if (!CachedInventory) return;

	const FLinearColor EmptyColor(0.12f, 0.12f, 0.15f, 0.9f);
	const FLinearColor OccupiedColor(0.25f, 0.2f, 0.1f, 0.9f);

	for (int32 y = 0; y < UInventoryComponent::GridHeight; ++y)
	{
		for (int32 x = 0; x < UInventoryComponent::GridWidth; ++x)
		{
			const int32 Idx = y * UInventoryComponent::GridWidth + x;
			if (Idx >= GridCellWidgets.Num()) continue;

			UBorder* Cell = GridCellWidgets[Idx];
			if (!Cell) continue;

			const FItemInstance* Item = CachedInventory->GetItemAt(x, y);
			Cell->SetBrushColor(Item ? OccupiedColor : EmptyColor);
			Cell->ClearChildren();

			if (Item && Item->Definition && Item->Definition->Icon)
			{
				UImage* IconImg = NewObject<UImage>(Cell);
				IconImg->SetBrushFromTexture(Item->Definition->Icon);
				IconImg->SetDesiredSizeOverride(FVector2D(CellSize - 4.f, CellSize - 4.f));
				Cell->AddChild(IconImg);
			}
			else if (Item && Item->Definition)
			{
				UTextBlock* NameLabel = NewObject<UTextBlock>(Cell);
				FString Short = Item->Definition->DisplayName.ToString().Left(3);
				NameLabel->SetText(FText::FromString(Short));
				FSlateFontInfo Font = NameLabel->GetFont();
				Font.Size = 8;
				NameLabel->SetFont(Font);
				NameLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
				Cell->AddChild(NameLabel);
			}
		}
	}
}

void UDiabloInventoryPanel::RefreshEquipment()
{
	if (!CachedInventory) return;

	const FLinearColor EmptyColor(0.1f, 0.1f, 0.18f, 0.9f);
	const FLinearColor EquippedColor(0.15f, 0.25f, 0.15f, 0.9f);

	for (auto& Pair : EquipSlotWidgets)
	{
		UBorder* Cell = Pair.Value;
		if (!Cell) continue;

		const FItemInstance& Equipped = CachedInventory->GetEquipped(Pair.Key);
		Cell->SetBrushColor(Equipped.IsValid() ? EquippedColor : EmptyColor);
		Cell->ClearChildren();

		if (Equipped.IsValid() && Equipped.Definition)
		{
			if (Equipped.Definition->Icon)
			{
				UImage* IconImg = NewObject<UImage>(Cell);
				IconImg->SetBrushFromTexture(Equipped.Definition->Icon);
				IconImg->SetDesiredSizeOverride(FVector2D(CellSize - 4.f, CellSize - 4.f));
				Cell->AddChild(IconImg);
			}
			else
			{
				UTextBlock* NameLabel = NewObject<UTextBlock>(Cell);
				FString Short = Equipped.Definition->DisplayName.ToString().Left(3);
				NameLabel->SetText(FText::FromString(Short));
				FSlateFontInfo Font = NameLabel->GetFont();
				Font.Size = 8;
				NameLabel->SetFont(Font);
				NameLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
				Cell->AddChild(NameLabel);
			}
		}
	}
}

void UDiabloInventoryPanel::RefreshGold()
{
	if (!CachedInventory || !GoldText) return;
	GoldText->SetText(FText::FromString(FString::Printf(TEXT("Gold: %d"), CachedInventory->GetGold())));
}

// ---------------------------------------------------------------------------
// Widget tree construction
// ---------------------------------------------------------------------------

static UTextBlock* MakeInvLabel(UWidgetTree* Tree, const FName& Name, const FString& Text, int32 FontSize = 12)
{
	UTextBlock* Block = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Block->SetText(FText::FromString(Text));
	Block->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.6f, 1.f)));

	FSlateFontInfo Font = Block->GetFont();
	Font.Size = FontSize;
	Block->SetFont(Font);

	return Block;
}

TSharedRef<SWidget> UDiabloInventoryPanel::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("InvRoot"));
		WidgetTree->RootWidget = Root;

		UBorder* BG = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InvBG"));
		BG->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.08f, 0.9f));
		BG->SetPadding(FMargin(10.f));

		const float GridW = UInventoryComponent::GridWidth * (CellSize + CellPadding);
		const float TotalWidth = GridW + 20.f;
		const float TotalHeight = 360.f;

		UCanvasPanelSlot* BGSlot = Root->AddChildToCanvas(BG);
		BGSlot->SetAnchors(FAnchors(1.f, 0.f, 1.f, 0.f));
		BGSlot->SetAlignment(FVector2D(1.f, 0.f));
		BGSlot->SetPosition(FVector2D(-10.f, 10.f));
		BGSlot->SetSize(FVector2D(TotalWidth, TotalHeight));

		UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InvVBox"));
		BG->AddChild(VBox);

		// Title
		TitleText = MakeInvLabel(WidgetTree, TEXT("InvTitle"), TEXT("Inventory"), 14);
		TitleText->SetJustification(ETextJustify::Center);
		VBox->AddChildToVerticalBox(TitleText)->SetPadding(FMargin(0.f, 0.f, 0.f, 2.f));

		// Hover item name
		HoverText = MakeInvLabel(WidgetTree, TEXT("HoverText"), TEXT(""), 11);
		HoverText->SetJustification(ETextJustify::Center);
		HoverText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.3f, 1.f)));
		VBox->AddChildToVerticalBox(HoverText)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

		// Equipment slots row
		{
			UHorizontalBox* EquipRow = WidgetTree->ConstructWidget<UHorizontalBox>(
				UHorizontalBox::StaticClass(), TEXT("EquipRow"));
			VBox->AddChildToVerticalBox(EquipRow)->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));

			struct FSlotInfo { EEquipSlot EquipSlotVal; FString Label; };
			TArray<FSlotInfo> Slots = {
				{ EEquipSlot::Head, TEXT("Head") },
				{ EEquipSlot::Chest, TEXT("Body") },
				{ EEquipSlot::LeftHand, TEXT("LH") },
				{ EEquipSlot::RightHand, TEXT("RH") },
				{ EEquipSlot::LeftRing, TEXT("LR") },
				{ EEquipSlot::RightRing, TEXT("RR") },
				{ EEquipSlot::Amulet, TEXT("Amu") },
			};

			for (const FSlotInfo& SI : Slots)
			{
				USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(),
					FName(*FString::Printf(TEXT("EquipBox_%s"), *SI.Label)));
				Box->SetWidthOverride(CellSize);
				Box->SetHeightOverride(CellSize);

				FName CellName(*FString::Printf(TEXT("Equip_%s"), *SI.Label));
				UBorder* Cell = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), CellName);
				Cell->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.18f, 0.9f));
				Cell->SetHorizontalAlignment(HAlign_Center);
				Cell->SetVerticalAlignment(VAlign_Center);

				UTextBlock* SlotLabel = WidgetTree->ConstructWidget<UTextBlock>(
					UTextBlock::StaticClass(), FName(*FString::Printf(TEXT("EquipLbl_%s"), *SI.Label)));
				SlotLabel->SetText(FText::FromString(SI.Label));
				FSlateFontInfo Font = SlotLabel->GetFont();
				Font.Size = 7;
				SlotLabel->SetFont(Font);
				SlotLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 0.6f)));
				Cell->AddChild(SlotLabel);

				Box->AddChild(Cell);
				EquipSlotWidgets.Add(SI.EquipSlotVal, Cell);

				UHorizontalBoxSlot* HSlot = EquipRow->AddChildToHorizontalBox(Box);
				HSlot->SetPadding(FMargin(CellPadding));
			}
		}

		// Separator
		{
			UBorder* Sep = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Separator"));
			Sep->SetBrushColor(FLinearColor(0.3f, 0.3f, 0.3f, 0.5f));
			USizeBox* SepBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SepBox"));
			SepBox->SetHeightOverride(2.f);
			SepBox->AddChild(Sep);
			VBox->AddChildToVerticalBox(SepBox)->SetPadding(FMargin(0.f, 2.f, 0.f, 4.f));
		}

		// Inventory grid
		GridCellWidgets.Empty();
		{
			UVerticalBox* GridVBox = WidgetTree->ConstructWidget<UVerticalBox>(
				UVerticalBox::StaticClass(), TEXT("GridVBox"));
			VBox->AddChildToVerticalBox(GridVBox);

			for (int32 y = 0; y < UInventoryComponent::GridHeight; ++y)
			{
				UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(
					UHorizontalBox::StaticClass(),
					FName(*FString::Printf(TEXT("GridRow_%d"), y)));

				for (int32 x = 0; x < UInventoryComponent::GridWidth; ++x)
				{
					USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(
						USizeBox::StaticClass(),
						FName(*FString::Printf(TEXT("GridBox_%d_%d"), x, y)));
					Box->SetWidthOverride(CellSize);
					Box->SetHeightOverride(CellSize);

					FName CellName(*FString::Printf(TEXT("Cell_%d_%d"), x, y));
					UBorder* Cell = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), CellName);
					Cell->SetBrushColor(FLinearColor(0.12f, 0.12f, 0.15f, 0.9f));
					Cell->SetHorizontalAlignment(HAlign_Center);
					Cell->SetVerticalAlignment(VAlign_Center);

					Box->AddChild(Cell);
					GridCellWidgets.Add(Cell);

					UHorizontalBoxSlot* HSlot = Row->AddChildToHorizontalBox(Box);
					HSlot->SetPadding(FMargin(CellPadding));
				}

				GridVBox->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.f, CellPadding));
			}
		}

		// Gold display
		{
			GoldText = MakeInvLabel(WidgetTree, TEXT("GoldText"), TEXT("Gold: 0"), 12);
			VBox->AddChildToVerticalBox(GoldText)->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
		}
	}

	return Super::RebuildWidget();
}
