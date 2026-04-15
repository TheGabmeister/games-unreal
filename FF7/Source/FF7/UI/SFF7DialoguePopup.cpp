// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/SFF7DialoguePopup.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FF7Dialogue"

void SFF7DialoguePopup::Construct(const FArguments& InArgs)
{
	SpeakerText = InArgs._SpeakerText;
	LineText = InArgs._LineText;

	ChildSlot
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	.Padding(FMargin(0.0f, 0.0f, 0.0f, 48.0f))
	[
		SNew(SBox)
		.WidthOverride(800.0f)
		.MinDesiredHeight(120.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
			.Padding(FMargin(16.0f, 12.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 0.0f, 0.0f, 6.0f))
				[
					SNew(STextBlock)
					.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.3f)))
					.Text(SpeakerText)
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(STextBlock)
					.ColorAndOpacity(FSlateColor(FLinearColor::White))
					.AutoWrapText(true)
					.Text(LineText)
				]
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE
