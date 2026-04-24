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
	FReply OnGenerateBlueprints();
	FReply OnGenerateMap();
	FReply OnGenerateInput();
	FReply OnImportWarrior();
	FReply OnImportAttackSFX();
	FReply OnImportPotion();
	FReply OnConfigureAll();
	FReply OnConfigureHero();
	FReply OnConfigureController();
	FReply OnConfigureGameMode();
	FReply OnConfigureEnemy();
};
