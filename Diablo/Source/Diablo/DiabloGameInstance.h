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
	UPROPERTY()
	bool bHasSavedState = false;

	UPROPERTY()
	FDiabloStats SavedStats;

	UPROPERTY()
	int32 SavedCharLevel = 1;

	UPROPERTY()
	int64 SavedCurrentXP = 0;

	UPROPERTY()
	int32 SavedUnspentStatPoints = 0;

	UPROPERTY()
	TArray<FItemInstance> SavedGridItems;

	UPROPERTY()
	TArray<int32> SavedOccupancyGrid;

	UPROPERTY()
	TMap<EEquipSlot, FItemInstance> SavedEquippedItems;

	UPROPERTY()
	int32 SavedGold = 0;

	UPROPERTY()
	TArray<FItemInstance> SavedBeltItems;

	UPROPERTY()
	TArray<TObjectPtr<USpellDefinition>> SavedKnownSpells;

	UPROPERTY()
	TObjectPtr<USpellDefinition> SavedActiveSpell;

	UPROPERTY()
	int32 CurrentFloorIndex = 0;
};
