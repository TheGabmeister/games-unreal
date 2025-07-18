#include "CBGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "CBEvents.h"
#include "GameplayTagContainer.h"
#include "GameData.h"

ACBGameMode::ACBGameMode()
{
	
}

void ACBGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	ListenerHandle = MessageSubsystem.RegisterListener(EV_OnPickedUp_WumpaFruit, this, &ThisClass::OnWumpaFruitPickedUp);
}

void ACBGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.UnregisterListener(ListenerHandle);
	
	Super::EndPlay(EndPlayReason);
}

void ACBGameMode::OnWumpaFruitPickedUp(FGameplayTag Channel, const FGameplayMessageInt& Message)
{
	UGameData* GameData = GetGameInstance()->GetSubsystem<UGameData>();
	GameData->UpdateWumpaFruit(Message.Value);
}
