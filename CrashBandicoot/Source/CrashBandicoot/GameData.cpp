#include "GameData.h"

void UGameData::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGameData::Deinitialize()
{
	Super::Deinitialize();
}


void UGameData::UpdateWumpaFruit(int32 Amount)
{
	WumpaFruit += Amount;
	OnWumpaFruitChanged.Broadcast(WumpaFruit);
}

void UGameData::UpdateLives(int32 Amount)
{
	Lives += Amount;
	OnLivesChanged.Broadcast(Lives);
}

void UGameData::UpdateAkuAkuCount(int32 Amount)
{
	AkuAkuCount += Amount;
	OnAkuAkuCountChanged.Broadcast(AkuAkuCount);
}
