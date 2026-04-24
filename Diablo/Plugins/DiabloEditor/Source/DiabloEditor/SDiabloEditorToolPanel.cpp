#include "SDiabloEditorToolPanel.h"
#include "DiabloAssetGenerator.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

void SDiabloEditorToolPanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(8.f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Diablo Asset Generator")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 8.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Generate All Assets")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnGenerateAll)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Generate Blueprint Subclasses")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnGenerateBlueprints)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Generate Default Map")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnGenerateMap)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Generate Input Assets")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnGenerateInput)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Import Warrior FBX")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnImportWarrior)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Import Attack SFX")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnImportAttackSFX)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Configure Blueprint Defaults")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnConfigureDefaults)
			]
		]
	];
}

FReply SDiabloEditorToolPanel::OnGenerateAll()
{
	FDiabloAssetGenerator::GenerateAllAssets();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnGenerateBlueprints()
{
	FDiabloAssetGenerator::GenerateBlueprintSubclasses();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnGenerateMap()
{
	FDiabloAssetGenerator::GenerateDefaultMap();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnGenerateInput()
{
	FDiabloAssetGenerator::GenerateInputAssets();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnImportWarrior()
{
	FDiabloAssetGenerator::ImportWarriorFBX();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnImportAttackSFX()
{
	FDiabloAssetGenerator::ImportAttackSFX();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnConfigureDefaults()
{
	FDiabloAssetGenerator::ConfigureBlueprintDefaults();
	return FReply::Handled();
}
