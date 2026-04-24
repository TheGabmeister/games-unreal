#include "DiabloHUDWidget.h"
#include "DiabloHero.h"
#include "Diablo.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"

void UDiabloHUDWidget::InitForHero(ADiabloHero* InHero)
{
	if (CachedHero && StatsChangedHandle.IsValid())
	{
		CachedHero->OnStatsChanged.Remove(StatsChangedHandle);
		StatsChangedHandle.Reset();
	}

	CachedHero = InHero;

	if (CachedHero)
	{
		StatsChangedHandle = CachedHero->OnStatsChanged.AddUObject(this, &UDiabloHUDWidget::OnStatsChanged);
		RefreshBars();
	}
}

void UDiabloHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshBars();
}

void UDiabloHUDWidget::NativeDestruct()
{
	if (CachedHero && StatsChangedHandle.IsValid())
	{
		CachedHero->OnStatsChanged.Remove(StatsChangedHandle);
		StatsChangedHandle.Reset();
	}

	Super::NativeDestruct();
}

void UDiabloHUDWidget::OnStatsChanged()
{
	RefreshBars();
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

		// --- Level Text (above XP bar, centered) ---
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
			LvSlot->SetPosition(FVector2D(0.f, -Margin - BarHeight - 4.f));
			LvSlot->SetAutoSize(true);
		}
	}

	return Super::RebuildWidget();
}
