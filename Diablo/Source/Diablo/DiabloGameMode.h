#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DiabloGameMode.generated.h"

UCLASS(Abstract)
class DIABLO_API ADiabloGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADiabloGameMode();

	int32 GetSeedForDungeonFloor(FName FloorName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 DungeonSeed = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dungeon")
	TMap<FName, int32> DungeonFloorSeeds;
};
