// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CBEvents.h"
#include "GameplayTagContainer.h"
#include "CBGameMode.generated.h"

UCLASS(minimalapi)
class ACBGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACBGameMode();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Handle for the message listener (to keep it alive)
	TSharedPtr<class FGameplayMessageSubscription> WumpaFruitMessageHandle;
	
	// Handler function for WumpaFruit pickup events
	void HandleWumpaFruitPickup(const FGameplayTag& MessageTag, const FGameplayMessageInt& Message);
};