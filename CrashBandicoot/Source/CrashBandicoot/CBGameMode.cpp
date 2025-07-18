
// Copyright Epic Games, Inc. All Rights Reserved.

#include "CBGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "CBEvents.h"
#include "GameFramework/GameplayMessageSubsystem.h"

ACBGameMode::ACBGameMode()
{
	
}

void ACBGameMode::BeginPlay()
{
	Super::BeginPlay();
    
	// Get the Gameplay Message Subsystem
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
    
	// Subscribe to the WumpaFruit pickup event using the correct delegate type
	WumpaFruitMessageHandle = MessageSubsystem.RegisterListener(
		Events_OnPickedUp_WumpaFruit, 
		UE::GameplayMessageSubsystem::FHandlerDelegate<FGameplayMessageInt>::CreateUObject(
			this, 
			&ACBGameMode::HandleWumpaFruitPickup
		)
	);

    
	UE_LOG(LogTemp, Log, TEXT("CBGameMode: Registered listener for WumpaFruit pickup events"));
}

void ACBGameMode::HandleWumpaFruitPickup(FGameplayTag MessageTag, const FGameplayMessageInt& Message)
{
	// Handle the WumpaFruit pickup event
	UE_LOG(LogTemp, Log, TEXT("WumpaFruit picked up! Value: %d"), Message.Value);
    
	// Add your game logic here for when a WumpaFruit is picked up
	// For example, update score, play sounds, etc.
}