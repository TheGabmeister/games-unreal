#include "DiabloGameMode.h"
#include "DiabloHero.h"
#include "DiabloPlayerController.h"

ADiabloGameMode::ADiabloGameMode()
{
	DefaultPawnClass = ADiabloHero::StaticClass();
	PlayerControllerClass = ADiabloPlayerController::StaticClass();
}

int32 ADiabloGameMode::GetSeedForDungeonFloor(FName FloorName)
{
	if (int32* ExistingSeed = DungeonFloorSeeds.Find(FloorName))
	{
		return *ExistingSeed;
	}

	const int32 BaseSeed = DungeonSeed != 0 ? DungeonSeed : FMath::Rand();
	const int32 FloorSeed = HashCombine(GetTypeHash(BaseSeed), GetTypeHash(FloorName));
	DungeonFloorSeeds.Add(FloorName, FloorSeed);
	return FloorSeed;
}
