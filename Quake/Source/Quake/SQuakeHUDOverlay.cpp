#include "SQuakeHUDOverlay.h"

#include "QuakeCharacter.h"

#include "Fonts/SlateFontInfo.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "QuakeHUDOverlay"

void SQuakeHUDOverlay::Construct(const FArguments& InArgs)
{
	PlayerCharacter = InArgs._PlayerCharacter;

	const FSlateFontInfo HealthFont = FCoreStyle::GetDefaultFontStyle("Bold", 36);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)  // Spacer pushes the health row to the bottom.
		[
			SNew(SBox)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(40.f, 0.f, 0.f, 40.f)
		[
			SNew(STextBlock)
			.Font(HealthFont)
			.ColorAndOpacity(FLinearColor(1.f, 0.85f, 0.4f))
			.Text(this, &SQuakeHUDOverlay::GetHealthText)
		]
	];
}

FText SQuakeHUDOverlay::GetHealthText() const
{
	if (const AQuakeCharacter* Char = PlayerCharacter.Get())
	{
		return FText::FromString(FString::Printf(TEXT("HP %d"), FMath::RoundToInt(Char->GetHealth())));
	}
	return FText::FromString(TEXT("HP --"));
}

#undef LOCTEXT_NAMESPACE
