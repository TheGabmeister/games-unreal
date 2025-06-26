// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameData.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWumpaFruitChanged, int32, NewWumpaFruit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLivesChanged, int32, NewLives);

UCLASS()
class CRASHBANDICOOT_API UGameData : public UGameInstanceSubsystem
{
	GENERATED_BODY()

    int32 WumpaFruit = 0;
	int32 Lives = 4;
	
	UPROPERTY(BlueprintAssignable)
	FOnWumpaFruitChanged OnWumpaFruitChanged;

	UPROPERTY(BlueprintAssignable)
	FOnLivesChanged OnLivesChanged;
	
	UFUNCTION(BlueprintCallable)
	int32 GetWumpaFruit() const { return WumpaFruit; }

	UFUNCTION(BlueprintCallable)
	int32 GetLives() const { return Lives; }
	
	UFUNCTION(BlueprintCallable)
	void UpdateWumpaFruit(int32 Amount);

	UFUNCTION(BlueprintCallable)
	void UpdateLives(int32 Amount);
};
