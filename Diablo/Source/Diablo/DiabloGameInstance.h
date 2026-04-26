#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DiabloStats.h"
#include "ItemInstance.h"
#include "DiabloGameInstance.generated.h"

class USpellDefinition;

UCLASS()
class DIABLO_API UDiabloGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	bool bHasSavedState = false;

	FDiabloStats SavedStats;
	int32 SavedCharLevel = 1;
	int64 SavedCurrentXP = 0;
	int32 SavedUnspentStatPoints = 0;

	TArray<FItemInstance> SavedGridItems;
	TArray<int32> SavedOccupancyGrid;
	TMap<EEquipSlot, FItemInstance> SavedEquippedItems;
	int32 SavedGold = 0;
	TArray<FItemInstance> SavedBeltItems;

	UPROPERTY()
	TArray<TObjectPtr<USpellDefinition>> SavedKnownSpells;

	UPROPERTY()
	TObjectPtr<USpellDefinition> SavedActiveSpell;
};
