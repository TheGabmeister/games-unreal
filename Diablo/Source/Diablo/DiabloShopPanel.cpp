#include "DiabloShopPanel.h"
#include "DiabloPlayerController.h"
#include "DiabloNPC.h"
#include "NPCShopData.h"
#include "InventoryComponent.h"
#include "ItemDefinition.h"
#include "AffixGenerator.h"
#include "DiabloHero.h"
#include "Diablo.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"

void UDiabloShopPanel::Init(ADiabloPlayerController* InController, ADiabloNPC* InNPC)
{
	if (PlayerInventory && InventoryChangedHandle.IsValid())
	{
		PlayerInventory->OnInventoryChanged.Remove(InventoryChangedHandle);
		InventoryChangedHandle.Reset();
	}

	OwnerController = InController;
	ShopNPC = InNPC;

	if (ADiabloHero* Hero = Cast<ADiabloHero>(InController->GetPawn()))
	{
		PlayerInventory = Hero->Inventory;
		if (PlayerInventory)
		{
			InventoryChangedHandle = PlayerInventory->OnInventoryChanged.AddUObject(
				this, &UDiabloShopPanel::OnInventoryChanged);
		}
	}

	RefreshShop();
}

void UDiabloShopPanel::NativeDestruct()
{
	if (PlayerInventory && InventoryChangedHandle.IsValid())
	{
		PlayerInventory->OnInventoryChanged.Remove(InventoryChangedHandle);
		InventoryChangedHandle.Reset();
	}
	Super::NativeDestruct();
}

void UDiabloShopPanel::OnInventoryChanged()
{
	RefreshShop();
}

static UBorder* MakeItemRow(UObject* Outer, const FString& ItemName, int32 Price, bool bCanAfford,
	const FLinearColor& NameColor = FLinearColor(0.9f, 0.85f, 0.6f, 1.f))
{
	UBorder* Row = NewObject<UBorder>(Outer);
	Row->SetBrushColor(FLinearColor(0.08f, 0.08f, 0.1f, 0.6f));
	Row->SetPadding(FMargin(6.f, 3.f));

	UHorizontalBox* HBox = NewObject<UHorizontalBox>(Outer);
	Row->AddChild(HBox);

	UTextBlock* NameLabel = NewObject<UTextBlock>(Outer);
	NameLabel->SetText(FText::FromString(ItemName));
	NameLabel->SetColorAndOpacity(FSlateColor(NameColor));
	FSlateFontInfo NameFont = NameLabel->GetFont();
	NameFont.Size = 11;
	NameLabel->SetFont(NameFont);
	UHorizontalBoxSlot* NameSlot = HBox->AddChildToHorizontalBox(NameLabel);
	NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	NameSlot->SetVerticalAlignment(VAlign_Center);

	UTextBlock* PriceLabel = NewObject<UTextBlock>(Outer);
	PriceLabel->SetText(FText::FromString(FString::Printf(TEXT("%dg"), Price)));
	FLinearColor PriceColor = bCanAfford ? FLinearColor(0.4f, 0.9f, 0.4f, 1.f) : FLinearColor(0.9f, 0.3f, 0.3f, 1.f);
	PriceLabel->SetColorAndOpacity(FSlateColor(PriceColor));
	FSlateFontInfo PriceFont = PriceLabel->GetFont();
	PriceFont.Size = 11;
	PriceLabel->SetFont(PriceFont);
	UHorizontalBoxSlot* PriceSlot = HBox->AddChildToHorizontalBox(PriceLabel);
	PriceSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	PriceSlot->SetVerticalAlignment(VAlign_Center);

	return Row;
}

void UDiabloShopPanel::RefreshShop()
{
	BuyRows.Empty();
	SellRows.Empty();
	SellEntries.Empty();

	if (BuyListBox)
	{
		BuyListBox->ClearChildren();
	}
	if (SellListBox)
	{
		SellListBox->ClearChildren();
	}

	const int32 PlayerGold = PlayerInventory ? PlayerInventory->GetGold() : 0;

	if (TitleText && ShopNPC)
	{
		TitleText->SetText(FText::FromString(ShopNPC->NPCName.ToString() + TEXT("'s Shop")));
	}
	if (GoldText)
	{
		GoldText->SetText(FText::FromString(FString::Printf(TEXT("Gold: %d"), PlayerGold)));
	}

	if (BuyListBox && ShopNPC && ShopNPC->ShopData)
	{
		for (int32 i = 0; i < ShopNPC->ShopData->StockItems.Num(); i++)
		{
			const UItemDefinition* Def = ShopNPC->ShopData->StockItems[i];
			if (!Def) continue;

			const int32 BuyPrice = Def->GoldValue;
			UBorder* Row = MakeItemRow(this, Def->DisplayName.ToString(), BuyPrice, PlayerGold >= BuyPrice);
			BuyListBox->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.f, 1.f));
			BuyRows.Add(Row);
		}
	}

	if (SellListBox && PlayerInventory)
	{
		const TArray<FItemInstance>& Items = PlayerInventory->GetGridItems();
		for (int32 i = 0; i < Items.Num(); i++)
		{
			if (!Items[i].IsValid()) continue;

			const int32 GridX = i % UInventoryComponent::GridWidth;
			const int32 GridY = i / UInventoryComponent::GridWidth;
			const int32 SellPrice = FMath::Max(1, FAffixGenerator::GetTotalGoldValue(Items[i]) / 4);

			const FLinearColor NameColor = Items[i].Affixes.Num() > 0
				? FLinearColor(0.3f, 0.3f, 1.f, 1.f)
				: FLinearColor(0.9f, 0.85f, 0.6f, 1.f);

			const FString DisplayName = FAffixGenerator::GetDisplayName(Items[i]);
			UBorder* Row = MakeItemRow(this, DisplayName, SellPrice, true, NameColor);
			SellListBox->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.f, 1.f));
			SellRows.Add(Row);
			SellEntries.Add({ GridX, GridY });
		}
	}
}

void UDiabloShopPanel::BuyItem(int32 StockIndex)
{
	if (!PlayerInventory || !ShopNPC || !ShopNPC->ShopData) return;
	if (StockIndex < 0 || StockIndex >= ShopNPC->ShopData->StockItems.Num()) return;

	const UItemDefinition* Def = ShopNPC->ShopData->StockItems[StockIndex];
	if (!Def) return;

	const int32 BuyPrice = Def->GoldValue;
	if (!PlayerInventory->SpendGold(BuyPrice)) return;

	FItemInstance NewItem;
	NewItem.Definition = const_cast<UItemDefinition*>(Def);
	NewItem.CurrentDurability = Def->MaxDurability;
	NewItem.StackCount = 1;

	if (!PlayerInventory->TryAddItem(NewItem))
	{
		PlayerInventory->AddGold(BuyPrice);
		UE_LOG(LogDiablo, Display, TEXT("Inventory full — purchase refunded"));
	}
}

void UDiabloShopPanel::SellItem(int32 SellIndex)
{
	if (!PlayerInventory) return;
	if (SellIndex < 0 || SellIndex >= SellEntries.Num()) return;

	const FSellEntry& Entry = SellEntries[SellIndex];
	const FItemInstance* Item = PlayerInventory->GetItemAt(Entry.GridX, Entry.GridY);
	if (!Item || !Item->IsValid()) return;

	const int32 SellPrice = FMath::Max(1, FAffixGenerator::GetTotalGoldValue(*Item) / 4);
	PlayerInventory->RemoveItemAt(Entry.GridX, Entry.GridY);
	PlayerInventory->AddGold(SellPrice);
}

TSharedRef<SWidget> UDiabloShopPanel::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("ShopRoot"));
		WidgetTree->RootWidget = Root;

		UBorder* BG = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ShopBG"));
		BG->SetBrushColor(FLinearColor(0.03f, 0.03f, 0.05f, 0.92f));
		BG->SetPadding(FMargin(14.f));

		UCanvasPanelSlot* BGSlot = Root->AddChildToCanvas(BG);
		BGSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		BGSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		BGSlot->SetPosition(FVector2D(0.f, 0.f));
		BGSlot->SetSize(FVector2D(500.f, 350.f));

		UVerticalBox* MainVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainVBox"));
		BG->AddChild(MainVBox);

		TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ShopTitle"));
		TitleText->SetText(FText::FromString(TEXT("Shop")));
		TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.6f, 1.f)));
		TitleText->SetJustification(ETextJustify::Center);
		FSlateFontInfo TitleFont = TitleText->GetFont();
		TitleFont.Size = 16;
		TitleText->SetFont(TitleFont);
		MainVBox->AddChildToVerticalBox(TitleText)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

		UHorizontalBox* Columns = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Columns"));
		UVerticalBoxSlot* ColSlot = MainVBox->AddChildToVerticalBox(Columns);
		ColSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		// Left column — Buy
		UVerticalBox* LeftCol = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LeftCol"));
		UHorizontalBoxSlot* LeftSlot = Columns->AddChildToHorizontalBox(LeftCol);
		LeftSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LeftSlot->SetPadding(FMargin(0.f, 0.f, 6.f, 0.f));

		UTextBlock* BuyHeader = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BuyHeader"));
		BuyHeader->SetText(FText::FromString(TEXT("For Sale")));
		BuyHeader->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.f)));
		FSlateFontInfo HeaderFont = BuyHeader->GetFont();
		HeaderFont.Size = 12;
		BuyHeader->SetFont(HeaderFont);
		LeftCol->AddChildToVerticalBox(BuyHeader)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

		BuyListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BuyList"));
		UVerticalBoxSlot* BuyListSlot = LeftCol->AddChildToVerticalBox(BuyListBox);
		BuyListSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		// Right column — Sell
		UVerticalBox* RightCol = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RightCol"));
		UHorizontalBoxSlot* RightSlot = Columns->AddChildToHorizontalBox(RightCol);
		RightSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RightSlot->SetPadding(FMargin(6.f, 0.f, 0.f, 0.f));

		UTextBlock* SellHeader = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SellHeader"));
		SellHeader->SetText(FText::FromString(TEXT("Your Items")));
		SellHeader->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.f)));
		SellHeader->SetFont(HeaderFont);
		RightCol->AddChildToVerticalBox(SellHeader)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

		SellListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SellList"));
		UVerticalBoxSlot* SellListSlot = RightCol->AddChildToVerticalBox(SellListBox);
		SellListSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		// Bottom — Gold + Close
		UHorizontalBox* BottomRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BottomRow"));
		MainVBox->AddChildToVerticalBox(BottomRow)->SetPadding(FMargin(0.f, 8.f, 0.f, 0.f));

		GoldText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GoldText"));
		GoldText->SetText(FText::FromString(TEXT("Gold: 0")));
		GoldText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.8f, 0.2f, 1.f)));
		FSlateFontInfo GoldFont = GoldText->GetFont();
		GoldFont.Size = 13;
		GoldText->SetFont(GoldFont);
		UHorizontalBoxSlot* GoldSlot = BottomRow->AddChildToHorizontalBox(GoldText);
		GoldSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		GoldSlot->SetVerticalAlignment(VAlign_Center);

		CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ShopCloseBtn"));
		FButtonStyle Style = CloseButton->GetStyle();
		Style.Normal.TintColor = FSlateColor(FLinearColor(0.15f, 0.12f, 0.08f, 0.9f));
		Style.Hovered.TintColor = FSlateColor(FLinearColor(0.3f, 0.25f, 0.15f, 0.9f));
		Style.Pressed.TintColor = FSlateColor(FLinearColor(0.1f, 0.08f, 0.05f, 0.9f));
		CloseButton->SetStyle(Style);

		UTextBlock* CloseLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ShopCloseLabel"));
		CloseLabel->SetText(FText::FromString(TEXT("Close")));
		CloseLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.6f, 1.f)));
		CloseLabel->SetJustification(ETextJustify::Center);
		FSlateFontInfo CloseFont = CloseLabel->GetFont();
		CloseFont.Size = 12;
		CloseLabel->SetFont(CloseFont);
		CloseButton->AddChild(CloseLabel);

		BottomRow->AddChildToHorizontalBox(CloseButton);
		CloseButton->OnClicked.AddDynamic(this, &UDiabloShopPanel::OnCloseClicked);
	}

	return Super::RebuildWidget();
}

FReply UDiabloShopPanel::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		const FVector2D ScreenPos = InMouseEvent.GetScreenSpacePosition();

		for (int32 i = 0; i < BuyRows.Num(); i++)
		{
			if (!BuyRows[i]) continue;
			const FGeometry RowGeom = BuyRows[i]->GetTickSpaceGeometry();
			const FVector2D LocalPos = RowGeom.AbsoluteToLocal(ScreenPos);
			const FVector2D Size = RowGeom.GetLocalSize();
			if (LocalPos.X >= 0 && LocalPos.Y >= 0 && LocalPos.X <= Size.X && LocalPos.Y <= Size.Y)
			{
				BuyItem(i);
				return FReply::Handled();
			}
		}

		for (int32 i = 0; i < SellRows.Num(); i++)
		{
			if (!SellRows[i]) continue;
			const FGeometry RowGeom = SellRows[i]->GetTickSpaceGeometry();
			const FVector2D LocalPos = RowGeom.AbsoluteToLocal(ScreenPos);
			const FVector2D Size = RowGeom.GetLocalSize();
			if (LocalPos.X >= 0 && LocalPos.Y >= 0 && LocalPos.X <= Size.X && LocalPos.Y <= Size.Y)
			{
				SellItem(i);
				return FReply::Handled();
			}
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UDiabloShopPanel::OnCloseClicked()
{
	if (OwnerController)
	{
		OwnerController->CloseShop();
	}
}
