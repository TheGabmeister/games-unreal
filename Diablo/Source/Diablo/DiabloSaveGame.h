#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DiabloStats.h"
#include "ItemInstance.h"
#include "DiabloSaveGame.generated.h"

class USpellDefinition;
class UDiabloGameInstance;

UCLASS()
class DIABLO_API UDiabloSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static const FString SaveSlotName;
	static const int32 UserIndex;

	void PopulateFromGameInstance(const UDiabloGameInstance* GI);
	void ApplyToGameInstance(UDiabloGameInstance* GI) const;

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
	bool bPortalActive = false;

	UPROPERTY()
	int32 PortalFloorIndex = 0;

	UPROPERTY()
	FVector PortalDungeonLocation = FVector::ZeroVector;

	UPROPERTY()
	int32 PortalDungeonSeed = 0;
};
