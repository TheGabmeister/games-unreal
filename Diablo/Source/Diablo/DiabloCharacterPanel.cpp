#include "DiabloCharacterPanel.h"
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

void UDiabloCharacterPanel::InitForHero(ADiabloHero* InHero)
{
	if (CachedHero && StatsChangedHandle.IsValid())
	{
		CachedHero->OnStatsChanged.Remove(StatsChangedHandle);
		StatsChangedHandle.Reset();
	}

	CachedHero = InHero;

	if (CachedHero)
	{
		StatsChangedHandle = CachedHero->OnStatsChanged.AddUObject(this, &UDiabloCharacterPanel::OnStatsChanged);
		RefreshDisplay();
	}
}

void UDiabloCharacterPanel::OnStatsChanged()
{
	RefreshDisplay();
}

void UDiabloCharacterPanel::RefreshDisplay()
{
	if (!CachedHero)
	{
		return;
	}

	const FDiabloStats& S = CachedHero->Stats;

	if (LevelText)
	{
		LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level %d"), CachedHero->CharLevel)));
	}

	if (PointsText)
	{
		FString Pts = FString::Printf(TEXT("Stat Points: %d"), CachedHero->UnspentStatPoints);
		PointsText->SetText(FText::FromString(Pts));
	}

	if (StrText) StrText->SetText(FText::FromString(FString::Printf(TEXT("Str: %.0f"), S.Str)));
	if (MagText) MagText->SetText(FText::FromString(FString::Printf(TEXT("Mag: %.0f"), S.Mag)));
	if (DexText) DexText->SetText(FText::FromString(FString::Printf(TEXT("Dex: %.0f"), S.Dex)));
	if (VitText) VitText->SetText(FText::FromString(FString::Printf(TEXT("Vit: %.0f"), S.Vit)));

	if (HPText) HPText->SetText(FText::FromString(FString::Printf(TEXT("HP: %.0f / %.0f"), S.HP, S.MaxHP)));
	if (ManaText) ManaText->SetText(FText::FromString(FString::Printf(TEXT("Mana: %.0f / %.0f"), S.Mana, S.MaxMana)));

	const bool bHasPoints = CachedHero->UnspentStatPoints > 0;
	if (StrButton) StrButton->SetVisibility(bHasPoints ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	if (MagButton) MagButton->SetVisibility(bHasPoints ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	if (DexButton) DexButton->SetVisibility(bHasPoints ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	if (VitButton) VitButton->SetVisibility(bHasPoints ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UDiabloCharacterPanel::OnStrPlus() { if (CachedHero) CachedHero->SpendStatPoint(FName("Str")); }
void UDiabloCharacterPanel::OnMagPlus() { if (CachedHero) CachedHero->SpendStatPoint(FName("Mag")); }
void UDiabloCharacterPanel::OnDexPlus() { if (CachedHero) CachedHero->SpendStatPoint(FName("Dex")); }
void UDiabloCharacterPanel::OnVitPlus() { if (CachedHero) CachedHero->SpendStatPoint(FName("Vit")); }

static UTextBlock* MakeLabel(UWidgetTree* Tree, const FName& Name, const FString& Initial, int32 FontSize = 12)
{
	UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Text->SetText(FText::FromString(Initial));
	Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.6f, 1.f)));

	FSlateFontInfo Font = Text->GetFont();
	Font.Size = FontSize;
	Text->SetFont(Font);

	return Text;
}

static UHorizontalBox* MakeStatRow(UWidgetTree* Tree, const FName& RowName,
	TObjectPtr<UTextBlock>& OutLabel, TObjectPtr<UButton>& OutButton, const FString& StatName)
{
	UHorizontalBox* Row = Tree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), RowName);

	OutLabel = MakeLabel(Tree, FName(*(StatName + TEXT("Text"))), StatName + TEXT(": 0"));

	UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(OutLabel);
	LabelSlot->SetPadding(FMargin(0.f, 0.f, 10.f, 0.f));
	LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	LabelSlot->SetVerticalAlignment(VAlign_Center);

	OutButton = Tree->ConstructWidget<UButton>(UButton::StaticClass(), FName(*(StatName + TEXT("Button"))));

	UTextBlock* PlusLabel = MakeLabel(Tree, FName(*(StatName + TEXT("Plus"))), TEXT("+"), 10);
	PlusLabel->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
	OutButton->AddChild(PlusLabel);

	UHorizontalBoxSlot* BtnSlot = Row->AddChildToHorizontalBox(OutButton);
	BtnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	BtnSlot->SetVerticalAlignment(VAlign_Center);

	return Row;
}

TSharedRef<SWidget> UDiabloCharacterPanel::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CharRoot"));
		WidgetTree->RootWidget = Root;

		UBorder* BG = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BG"));
		BG->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.08f, 0.85f));
		BG->SetPadding(FMargin(12.f));

		UCanvasPanelSlot* BGSlot = Root->AddChildToCanvas(BG);
		BGSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		BGSlot->SetAlignment(FVector2D(0.f, 0.f));
		BGSlot->SetPosition(FVector2D(10.f, 10.f));
		BGSlot->SetSize(FVector2D(220.f, 280.f));

		UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("VBox"));
		BG->AddChild(VBox);

		TitleText = MakeLabel(WidgetTree, TEXT("TitleText"), TEXT("Character"), 16);
		TitleText->SetJustification(ETextJustify::Center);
		UVerticalBoxSlot* TitleSlot = VBox->AddChildToVerticalBox(TitleText);
		TitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

		LevelText = MakeLabel(WidgetTree, TEXT("LvlText"), TEXT("Level 1"));
		VBox->AddChildToVerticalBox(LevelText)->SetPadding(FMargin(0.f, 0.f, 0.f, 2.f));

		PointsText = MakeLabel(WidgetTree, TEXT("PtsText"), TEXT("Stat Points: 0"));
		VBox->AddChildToVerticalBox(PointsText)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

		// Stat rows
		UHorizontalBox* StrRow = MakeStatRow(WidgetTree, TEXT("StrRow"), StrText, StrButton, TEXT("Str"));
		VBox->AddChildToVerticalBox(StrRow)->SetPadding(FMargin(0.f, 2.f));

		UHorizontalBox* MagRow = MakeStatRow(WidgetTree, TEXT("MagRow"), MagText, MagButton, TEXT("Mag"));
		VBox->AddChildToVerticalBox(MagRow)->SetPadding(FMargin(0.f, 2.f));

		UHorizontalBox* DexRow = MakeStatRow(WidgetTree, TEXT("DexRow"), DexText, DexButton, TEXT("Dex"));
		VBox->AddChildToVerticalBox(DexRow)->SetPadding(FMargin(0.f, 2.f));

		UHorizontalBox* VitRow = MakeStatRow(WidgetTree, TEXT("VitRow"), VitText, VitButton, TEXT("Vit"));
		VBox->AddChildToVerticalBox(VitRow)->SetPadding(FMargin(0.f, 2.f, 0.f, 8.f));

		// HP / Mana
		HPText = MakeLabel(WidgetTree, TEXT("HPText"), TEXT("HP: 70 / 70"));
		VBox->AddChildToVerticalBox(HPText)->SetPadding(FMargin(0.f, 2.f));

		ManaText = MakeLabel(WidgetTree, TEXT("ManaText"), TEXT("Mana: 10 / 10"));
		VBox->AddChildToVerticalBox(ManaText)->SetPadding(FMargin(0.f, 2.f));

		// Bind button clicks
		StrButton->OnClicked.AddDynamic(this, &UDiabloCharacterPanel::OnStrPlus);
		MagButton->OnClicked.AddDynamic(this, &UDiabloCharacterPanel::OnMagPlus);
		DexButton->OnClicked.AddDynamic(this, &UDiabloCharacterPanel::OnDexPlus);
		VitButton->OnClicked.AddDynamic(this, &UDiabloCharacterPanel::OnVitPlus);
	}

	return Super::RebuildWidget();
}
