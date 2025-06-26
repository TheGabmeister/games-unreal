// Fill out your copyright notice in the Description page of Project Settings.


#include "GameData.h"

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