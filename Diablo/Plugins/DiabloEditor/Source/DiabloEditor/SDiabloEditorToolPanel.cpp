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
				.Text(FText::FromString(TEXT("Import Potion Sprite")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnImportPotion)
			]

			// --- Configure Defaults ---

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 12.f, 0.f, 4.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Configure Blueprint Defaults")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Configure All Defaults")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnConfigureAll)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Configure Hero")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnConfigureHero)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Configure Controller")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnConfigureController)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Configure Game Mode")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnConfigureGameMode)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Configure Enemy")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnConfigureEnemy)
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

FReply SDiabloEditorToolPanel::OnImportPotion()
{
	FDiabloAssetGenerator::ImportPotionSprite();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnConfigureAll()
{
	FDiabloAssetGenerator::ConfigureBlueprintDefaults();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnConfigureHero()
{
	FDiabloAssetGenerator::ConfigureHeroDefaults();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnConfigureController()
{
	FDiabloAssetGenerator::ConfigureControllerDefaults();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnConfigureGameMode()
{
	FDiabloAssetGenerator::ConfigureGameModeDefaults();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnConfigureEnemy()
{
	FDiabloAssetGenerator::ConfigureEnemyDefaults();
	return FReply::Handled();
}
