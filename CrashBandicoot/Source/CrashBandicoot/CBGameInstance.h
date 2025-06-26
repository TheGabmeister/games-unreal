// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CBGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class CRASHBANDICOOT_API UCBGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	virtual void Init() override;
};
