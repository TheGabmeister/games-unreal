#include "DiabloSpellbookPanel.h"
#include "DiabloHero.h"
#include "SpellDefinition.h"
#include "Diablo.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"

void UDiabloSpellbookPanel::InitForHero(ADiabloHero* InHero)
{
	CachedHero = InHero;
	RefreshDisplay();
}

void UDiabloSpellbookPanel::RefreshDisplay()
{
	if (!CachedHero || !SpellListBox)
	{
		return;
	}

	SpellListBox->ClearChildren();
	SpellLabels.Empty();

	for (int32 i = 0; i < CachedHero->KnownSpells.Num(); ++i)
	{
		USpellDefinition* Spell = CachedHero->KnownSpells[i];
		if (!Spell) continue;

		FString Label = FString::Printf(TEXT("%s  (Mana: %.0f)"),
			*Spell->DisplayName.ToString(), Spell->ManaCost);

		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), *FString::Printf(TEXT("Spell_%d"), i));
		Text->SetText(FText::FromString(Label));

		bool bIsActive = (CachedHero->GetActiveSpell() == Spell);
		FLinearColor Color = bIsActive
			? FLinearColor(1.f, 0.85f, 0.2f, 1.f)
			: FLinearColor(0.9f, 0.85f, 0.6f, 1.f);
		Text->SetColorAndOpacity(FSlateColor(Color));

		FSlateFontInfo Font = Text->GetFont();
		Font.Size = 12;
		Text->SetFont(Font);

		UVerticalBoxSlot* VSlot = SpellListBox->AddChildToVerticalBox(Text);
		VSlot->SetPadding(FMargin(0.f, 4.f));

		SpellLabels.Add(Text);
	}

	if (ActiveSpellText)
	{
		USpellDefinition* Active = CachedHero->GetActiveSpell();
		FString ActiveStr = Active
			? FString::Printf(TEXT("RMB: %s"), *Active->DisplayName.ToString())
			: TEXT("RMB: None");
		ActiveSpellText->SetText(FText::FromString(ActiveStr));
	}
}

int32 UDiabloSpellbookPanel::GetSpellIndexAtPosition(const FGeometry& InGeometry, const FVector2D& ScreenPosition) const
{
	const FVector2D Local = InGeometry.AbsoluteToLocal(ScreenPosition);
	const float Y = Local.Y - HeaderHeight;
	if (Y < 0.f) return INDEX_NONE;

	const int32 Index = static_cast<int32>(Y / RowHeight);
	if (!CachedHero || Index < 0 || Index >= CachedHero->KnownSpells.Num())
	{
		return INDEX_NONE;
	}
	return Index;
}

FReply UDiabloSpellbookPanel::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton && CachedHero)
	{
		const int32 Index = GetSpellIndexAtPosition(InGeometry, InMouseEvent.GetScreenSpacePosition());
		if (Index != INDEX_NONE)
		{
			CachedHero->SetActiveSpell(CachedHero->KnownSpells[Index]);
			RefreshDisplay();
			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

TSharedRef<SWidget> UDiabloSpellbookPanel::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(
			UCanvasPanel::StaticClass(), TEXT("SpellbookRoot"));
		WidgetTree->RootWidget = Root;

		UBorder* BG = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BG"));
		BG->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.08f, 0.85f));
		BG->SetPadding(FMargin(12.f));

		UCanvasPanelSlot* BGSlot = Root->AddChildToCanvas(BG);
		BGSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		BGSlot->SetAlignment(FVector2D(0.f, 0.f));
		BGSlot->SetPosition(FVector2D(240.f, 10.f));
		BGSlot->SetSize(FVector2D(250.f, 320.f));

		UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("VBox"));
		BG->AddChild(VBox);

		TitleText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("TitleText"));
		TitleText->SetText(FText::FromString(TEXT("Spellbook")));
		TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.6f, 1.f)));
		TitleText->SetJustification(ETextJustify::Center);
		FSlateFontInfo TitleFont = TitleText->GetFont();
		TitleFont.Size = 16;
		TitleText->SetFont(TitleFont);
		VBox->AddChildToVerticalBox(TitleText)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

		ActiveSpellText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("ActiveText"));
		ActiveSpellText->SetText(FText::FromString(TEXT("RMB: None")));
		ActiveSpellText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.2f, 1.f)));
		FSlateFontInfo ActiveFont = ActiveSpellText->GetFont();
		ActiveFont.Size = 11;
		ActiveSpellText->SetFont(ActiveFont);
		VBox->AddChildToVerticalBox(ActiveSpellText)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

		SpellListBox = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("SpellList"));
		VBox->AddChildToVerticalBox(SpellListBox);
	}

	return Super::RebuildWidget();
}
