
// Copyright Epic Games, Inc. All Rights Reserved.

#include "CBGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "CBEvents.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayTagContainer.h"
#include "GameFramework/AsyncAction_ListenForGameplayMessage.h"

ACBGameMode::ACBGameMode()
{
	
}

void ACBGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	UAsyncAction_ListenForGameplayMessage* Listener = UAsyncAction_ListenForGameplayMessage::ListenForGameplayMessages(
		this,
		Events_OnPickedUp_WumpaFruit,
		FGameplayMessageInt::StaticStruct(),
		EGameplayMessageMatch::ExactMatch
	);
	if (Listener)
	{
		Listener->OnMessageReceived.AddDynamic(this, &ThisClass::OnGameplayMessageReceived);
		Listener->Activate();
	}
}

void ACBGameMode::OnGameplayMessageReceived(UAsyncAction_ListenForGameplayMessage* Proxy, FGameplayTag ActualChannel)
{
	FGameplayMessageInt Payload;
	if (Proxy->GetPayload(Payload.Value))
	{
		UE_LOG(LogTemp, Log, TEXT("CBGameMode: Payload WumpaFruit pickup events: %d"), Payload.Value);
	}
}