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

			// --- Import ---

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 12.f, 0.f, 4.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Import")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
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
				.Text(FText::FromString(TEXT("Import Level-Up SFX")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnImportLevelUpSFX)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Import Potion Sprite")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnImportPotion)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Import Item Icons")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnImportItemIcons)
			]

			// --- Blueprints ---

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 12.f, 0.f, 4.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Blueprints")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Setup Hero")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnSetupHero)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Setup Controller")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnSetupController)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Setup Game Mode")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnSetupGameMode)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Setup Enemy")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnSetupEnemy)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Setup Potion")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnSetupPotion)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Setup HUD")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnSetupHUD)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Setup Inventory")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnSetupInventory)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Setup Drop Material")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnSetupDropMaterial)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Setup Spells")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnSetupSpells)
			]

			// --- World ---

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 12.f, 0.f, 4.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("World")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
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
				.Text(FText::FromString(TEXT("Generate Cathedral Map")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnGenerateCathedral)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Generate Input Assets")))
				.OnClicked(this, &SDiabloEditorToolPanel::OnGenerateInput)
			]
		]
	];
}

FReply SDiabloEditorToolPanel::OnGenerateAll()
{
	FDiabloAssetGenerator::GenerateAllAssets();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnGenerateMap()
{
	FDiabloAssetGenerator::GenerateDefaultMap();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnGenerateCathedral()
{
	FDiabloAssetGenerator::GenerateCathedralMap();
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

FReply SDiabloEditorToolPanel::OnImportLevelUpSFX()
{
	FDiabloAssetGenerator::ImportLevelUpSFX();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnImportPotion()
{
	FDiabloAssetGenerator::ImportPotionSprite();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnImportItemIcons()
{
	FDiabloAssetGenerator::ImportItemIcons();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnSetupHero()
{
	FDiabloAssetGenerator::SetupHero();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnSetupController()
{
	FDiabloAssetGenerator::SetupController();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnSetupGameMode()
{
	FDiabloAssetGenerator::SetupGameMode();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnSetupEnemy()
{
	FDiabloAssetGenerator::SetupEnemy();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnSetupPotion()
{
	FDiabloAssetGenerator::SetupPotion();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnSetupHUD()
{
	FDiabloAssetGenerator::SetupHUD();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnSetupInventory()
{
	FDiabloAssetGenerator::SetupInventory();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnSetupDropMaterial()
{
	FDiabloAssetGenerator::SetupDropMaterial();
	return FReply::Handled();
}

FReply SDiabloEditorToolPanel::OnSetupSpells()
{
	FDiabloAssetGenerator::SetupSpells();
	return FReply::Handled();
}
