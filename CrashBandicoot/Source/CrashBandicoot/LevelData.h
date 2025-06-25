// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelData.generated.h"

USTRUCT()
struct FLevelData : public FTableRowBase
{
	GENERATED_BODY()
				 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> Level;
				 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* Music;
};