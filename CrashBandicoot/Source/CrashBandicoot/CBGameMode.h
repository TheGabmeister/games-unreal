#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "CBGameMode.generated.h"

struct FGameplayTag;
struct FGameplayMessageInt;

UCLASS(minimalapi)
class ACBGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACBGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FGameplayMessageListenerHandle ListenerHandle;
	void OnWumpaFruitPickedUp(FGameplayTag Channel, const FGameplayMessageInt& Message);
};