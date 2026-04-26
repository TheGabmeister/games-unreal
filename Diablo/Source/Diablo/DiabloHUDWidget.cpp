#include "DiabloHUDWidget.h"
#include "DiabloHero.h"
#include "InventoryComponent.h"
#include "ItemDefinition.h"
#include "ItemInstance.h"
#include "Diablo.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/WidgetTree.h"

void UDiabloHUDWidget::InitForHero(ADiabloHero* InHero)
{
	if (CachedHero && StatsChangedHandle.IsValid())
	{
		CachedHero->OnStatsChanged.Remove(StatsChangedHandle);
		StatsChangedHandle.Reset();
	}
	if (CachedInventory && InventoryChangedHandle.IsValid())
	{
		CachedInventory->OnInventoryChanged.Remove(InventoryChangedHandle);
		InventoryChangedHandle.Reset();
	}

	CachedHero = InHero;
	CachedInventory = InHero ? InHero->Inventory : nullptr;

	if (CachedHero)
	{
		StatsChangedHandle = CachedHero->OnStatsChanged.AddUObject(this, &UDiabloHUDWidget::OnStatsChanged);
		RefreshBars();
	}
	if (CachedInventory)
	{
		InventoryChangedHandle = CachedInventory->OnInventoryChanged.AddUObject(this, &UDiabloHUDWidget::OnInventoryChanged);
		RefreshBelt();
	}
}

void UDiabloHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshBars();
	RefreshBelt();
}

void UDiabloHUDWidget::NativeDestruct()
{
	if (CachedHero && StatsChangedHandle.IsValid())
	{
		CachedHero->OnStatsChanged.Remove(StatsChangedHandle);
		StatsChangedHandle.Reset();
	}
	if (CachedInventory && InventoryChangedHandle.IsValid())
	{
		CachedInventory->OnInventoryChanged.Remove(InventoryChangedHandle);
		InventoryChangedHandle.Reset();
	}

	Super::NativeDestruct();
}

void UDiabloHUDWidget::OnStatsChanged()
{
	RefreshBars();
}

void UDiabloHUDWidget::OnInventoryChanged()
{
	RefreshBelt();
}

void UDiabloHUDWidget::RefreshBelt()
{
	if (!CachedInventory) return;

	const FLinearColor EmptyColor(0.08f, 0.08f, 0.1f, 0.7f);
	const FLinearColor FilledColor(0.2f, 0.18f, 0.1f, 0.85f);

	for (int32 i = 0; i < BeltSlotWidgets.Num(); ++i)
	{
		UBorder* Cell = BeltSlotWidgets[i];
		if (!Cell) continue;

		const FItemInstance& Item = CachedInventory->GetBeltItem(i);
		Cell->SetBrushColor(Item.IsValid() ? FilledColor : EmptyColor);
		Cell->ClearChildren();

		if (Item.IsValid() && Item.Definition)
		{
			if (Item.Definition->Icon)
			{
				UImage* IconImg = NewObject<UImage>(Cell);
				IconImg->SetBrushFromTexture(Item.Definition->Icon);
				IconImg->SetDesiredSizeOverride(FVector2D(28.f, 28.f));
				Cell->AddChild(IconImg);
			}
			else
			{
				UTextBlock* Label = NewObject<UTextBlock>(Cell);
				Label->SetText(FText::FromString(Item.Definition->DisplayName.ToString().Left(3)));
				FSlateFontInfo Font = Label->GetFont();
				Font.Size = 8;
				Label->SetFont(Font);
				Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
				Cell->AddChild(Label);
			}
		}
	}
}

void UDiabloHUDWidget::RefreshBars()
{
	if (!CachedHero)
	{
		return;
	}

	const FDiabloStats& S = CachedHero->Stats;

	if (LifeBar)
	{
		LifeBar->SetPercent(S.MaxHP > 0.f ? S.HP / S.MaxHP : 0.f);
	}

	if (ManaBar)
	{
		ManaBar->SetPercent(S.MaxMana > 0.f ? S.Mana / S.MaxMana : 0.f);
	}

	if (XPBar)
	{
		XPBar->SetPercent(CachedHero->GetXPPercent());
	}

	if (LevelText)
	{
		LevelText->SetText(FText::FromString(FString::Printf(TEXT("Lv %d"), CachedHero->CharLevel)));
	}
}

static UProgressBar* CreateGlobeBar(UWidgetTree* Tree, const FName& Name,
	const FLinearColor& FillColor, const FLinearColor& BgColor)
{
	UProgressBar* Bar = Tree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), Name);
	Bar->SetFillColorAndOpacity(FillColor);
	Bar->SetPercent(1.f);
	Bar->SetBarFillType(EProgressBarFillType::BottomToTop);
	Bar->SetBarFillStyle(EProgressBarFillStyle::Scale);

	FProgressBarStyle Style;
	FSlateBrush FillBrush;
	FillBrush.TintColor = FSlateColor(FillColor);
	Style.SetFillImage(FillBrush);

	FSlateBrush BgBrush;
	BgBrush.TintColor = FSlateColor(BgColor);
	Style.SetBackgroundImage(BgBrush);

	Bar->SetWidgetStyle(Style);

	return Bar;
}

TSharedRef<SWidget> UDiabloHUDWidget::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;

		const float GlobeSize = 80.f;
		const float BarHeight = 12.f;
		const float Margin = 10.f;

		// --- Life Globe (bottom-left) ---
		{
			USizeBox* LifeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("LifeBox"));
			LifeBox->SetWidthOverride(GlobeSize);
			LifeBox->SetHeightOverride(GlobeSize);

			LifeBar = CreateGlobeBar(WidgetTree, TEXT("LifeBar"),
				FLinearColor(0.7f, 0.05f, 0.05f, 1.f),
				FLinearColor(0.15f, 0.02f, 0.02f, 1.f));

			LifeBox->AddChild(LifeBar);

			UCanvasPanelSlot* LifeSlot = Root->AddChildToCanvas(LifeBox);
			LifeSlot->SetAnchors(FAnchors(0.f, 1.f, 0.f, 1.f));
			LifeSlot->SetAlignment(FVector2D(0.f, 1.f));
			LifeSlot->SetPosition(FVector2D(Margin, -Margin));
			LifeSlot->SetSize(FVector2D(GlobeSize, GlobeSize));
		}

		// --- Mana Globe (bottom-right) ---
		{
			USizeBox* ManaBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ManaBox"));
			ManaBox->SetWidthOverride(GlobeSize);
			ManaBox->SetHeightOverride(GlobeSize);

			ManaBar = CreateGlobeBar(WidgetTree, TEXT("ManaBar"),
				FLinearColor(0.05f, 0.1f, 0.7f, 1.f),
				FLinearColor(0.02f, 0.02f, 0.15f, 1.f));

			ManaBox->AddChild(ManaBar);

			UCanvasPanelSlot* ManaSlot = Root->AddChildToCanvas(ManaBox);
			ManaSlot->SetAnchors(FAnchors(1.f, 1.f, 1.f, 1.f));
			ManaSlot->SetAlignment(FVector2D(1.f, 1.f));
			ManaSlot->SetPosition(FVector2D(-Margin, -Margin));
			ManaSlot->SetSize(FVector2D(GlobeSize, GlobeSize));
		}

		// --- XP Bar (bottom-center, between the two globes) ---
		{
			XPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("XPBar"));
			XPBar->SetPercent(0.f);
			XPBar->SetBarFillType(EProgressBarFillType::LeftToRight);
			XPBar->SetBarFillStyle(EProgressBarFillStyle::Scale);

			FProgressBarStyle XPStyle;
			FSlateBrush XPFill;
			XPFill.TintColor = FSlateColor(FLinearColor(0.8f, 0.7f, 0.1f, 1.f));
			XPStyle.SetFillImage(XPFill);

			FSlateBrush XPBg;
			XPBg.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.8f));
			XPStyle.SetBackgroundImage(XPBg);

			XPBar->SetWidgetStyle(XPStyle);

			const float XPInset = Margin + GlobeSize + Margin;
			UCanvasPanelSlot* XPSlot = Root->AddChildToCanvas(XPBar);
			XPSlot->SetAnchors(FAnchors(0.f, 1.f, 1.f, 1.f));
			XPSlot->SetAutoSize(false);
			XPSlot->SetOffsets(FMargin(XPInset, -Margin - BarHeight, XPInset, Margin));
		}

		// --- Belt Slots (above XP bar, centered) ---
		{
			const float BeltCellSize = 32.f;
			const float BeltPadding = 2.f;
			const int32 NumBeltSlots = 8;

			UHorizontalBox* BeltRow = WidgetTree->ConstructWidget<UHorizontalBox>(
				UHorizontalBox::StaticClass(), TEXT("BeltRow"));

			BeltSlotWidgets.Empty();
			for (int32 i = 0; i < NumBeltSlots; ++i)
			{
				USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(
					USizeBox::StaticClass(),
					FName(*FString::Printf(TEXT("BeltBox_%d"), i)));
				Box->SetWidthOverride(BeltCellSize);
				Box->SetHeightOverride(BeltCellSize);

				UBorder* Cell = WidgetTree->ConstructWidget<UBorder>(
					UBorder::StaticClass(),
					FName(*FString::Printf(TEXT("BeltCell_%d"), i)));
				Cell->SetBrushColor(FLinearColor(0.08f, 0.08f, 0.1f, 0.7f));
				Cell->SetHorizontalAlignment(HAlign_Center);
				Cell->SetVerticalAlignment(VAlign_Center);

				UTextBlock* NumLabel = WidgetTree->ConstructWidget<UTextBlock>(
					UTextBlock::StaticClass(),
					FName(*FString::Printf(TEXT("BeltNum_%d"), i)));
				NumLabel->SetText(FText::FromString(FString::Printf(TEXT("%d"), i + 1)));
				FSlateFontInfo NumFont = NumLabel->GetFont();
				NumFont.Size = 7;
				NumLabel->SetFont(NumFont);
				NumLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.4f, 0.4f, 0.5f)));
				Cell->AddChild(NumLabel);

				Box->AddChild(Cell);
				BeltSlotWidgets.Add(Cell);

				UHorizontalBoxSlot* HSlot = BeltRow->AddChildToHorizontalBox(Box);
				HSlot->SetPadding(FMargin(BeltPadding));
			}

			UCanvasPanelSlot* BeltSlot = Root->AddChildToCanvas(BeltRow);
			BeltSlot->SetAnchors(FAnchors(0.5f, 1.f, 0.5f, 1.f));
			BeltSlot->SetAlignment(FVector2D(0.5f, 1.f));
			BeltSlot->SetPosition(FVector2D(0.f, -Margin - BarHeight - 6.f));
			BeltSlot->SetAutoSize(true);
		}

		// --- Level Text (above belt, centered) ---
		{
			LevelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LevelText"));
			LevelText->SetText(FText::FromString(TEXT("Lv 1")));
			LevelText->SetJustification(ETextJustify::Center);

			FSlateFontInfo Font = LevelText->GetFont();
			Font.Size = 14;
			LevelText->SetFont(Font);
			LevelText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.6f, 1.f)));

			UCanvasPanelSlot* LvSlot = Root->AddChildToCanvas(LevelText);
			LvSlot->SetAnchors(FAnchors(0.5f, 1.f, 0.5f, 1.f));
			LvSlot->SetAlignment(FVector2D(0.5f, 1.f));
			LvSlot->SetPosition(FVector2D(0.f, -Margin - BarHeight - 6.f - 36.f));
			LvSlot->SetAutoSize(true);
		}
	}

	return Super::RebuildWidget();
}
