

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CBGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

UCLASS()
class CRASHBANDICOOT_API ACBGameState : public AGameStateBase
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDied OnPlayerDied;
	UFUNCTION(BlueprintCallable, Category = "Events")
	void CallPlayerDied() { OnPlayerDied.Broadcast(); }
	
};
