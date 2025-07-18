// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CBGameMode.generated.h"

class UAsyncAction_ListenForGameplayMessage;
struct FGameplayTag;

UCLASS(minimalapi)
class ACBGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACBGameMode();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnGameplayMessageReceived(UAsyncAction_ListenForGameplayMessage* Proxy, FGameplayTag ActualChannel);
};