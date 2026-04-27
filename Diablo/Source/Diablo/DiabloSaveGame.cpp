#include "DiabloSaveGame.h"
#include "DiabloGameInstance.h"

const FString UDiabloSaveGame::SaveSlotName = TEXT("DiabloSave");
const int32 UDiabloSaveGame::UserIndex = 0;

void UDiabloSaveGame::PopulateFromGameInstance(const UDiabloGameInstance* GI)
{
	SavedStats = GI->SavedStats;
	SavedCharLevel = GI->SavedCharLevel;
	SavedCurrentXP = GI->SavedCurrentXP;
	SavedUnspentStatPoints = GI->SavedUnspentStatPoints;
	SavedGridItems = GI->SavedGridItems;
	SavedOccupancyGrid = GI->SavedOccupancyGrid;
	SavedEquippedItems = GI->SavedEquippedItems;
	SavedGold = GI->SavedGold;
	SavedBeltItems = GI->SavedBeltItems;
	SavedKnownSpells = GI->SavedKnownSpells;
	SavedActiveSpell = GI->SavedActiveSpell;
	bPortalActive = GI->bPortalActive;
	PortalFloorIndex = GI->PortalFloorIndex;
	PortalDungeonLocation = GI->PortalDungeonLocation;
	PortalDungeonSeed = GI->PortalDungeonSeed;
}

void UDiabloSaveGame::ApplyToGameInstance(UDiabloGameInstance* GI) const
{
	GI->bHasSavedState = true;
	GI->SavedStats = SavedStats;
	GI->SavedCharLevel = SavedCharLevel;
	GI->SavedCurrentXP = SavedCurrentXP;
	GI->SavedUnspentStatPoints = SavedUnspentStatPoints;
	GI->SavedGridItems = SavedGridItems;
	GI->SavedOccupancyGrid = SavedOccupancyGrid;
	GI->SavedEquippedItems = SavedEquippedItems;
	GI->SavedGold = SavedGold;
	GI->SavedBeltItems = SavedBeltItems;
	GI->SavedKnownSpells = SavedKnownSpells;
	GI->SavedActiveSpell = SavedActiveSpell;
	GI->bPortalActive = bPortalActive;
	GI->PortalFloorIndex = PortalFloorIndex;
	GI->PortalDungeonLocation = PortalDungeonLocation;
	GI->PortalDungeonSeed = PortalDungeonSeed;
}
