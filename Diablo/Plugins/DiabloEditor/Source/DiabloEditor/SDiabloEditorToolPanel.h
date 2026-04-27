#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SDiabloEditorToolPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDiabloEditorToolPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply OnGenerateAll();
	FReply OnGenerateMap();
	FReply OnGenerateCathedral();
	FReply OnGenerateDungeon();
	FReply OnGenerateDebugCombat();
	FReply OnGenerateInput();
	FReply OnImportWarrior();
	FReply OnImportAttackSFX();
	FReply OnImportLevelUpSFX();
	FReply OnImportPotion();
	FReply OnImportItemIcons();
	FReply OnSetupHero();
	FReply OnSetupController();
	FReply OnSetupGameMode();
	FReply OnSetupEnemy();
	FReply OnSetupPotion();
	FReply OnSetupHUD();
	FReply OnSetupInventory();
	FReply OnSetupDropMaterial();
	FReply OnSetupSpells();
	FReply OnSetupShopData();
	FReply OnSetupAffixes();
	FReply OnSetupDungeonPalette();
	FReply OnSetupDungeonPalettes();
};
