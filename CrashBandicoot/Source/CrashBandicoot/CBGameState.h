

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CBGameState.generated.h"

UENUM(BlueprintType)
enum class ELevelState : uint8
{
	Preload,
	Ready,
	Running,
	Paused,
	GameOver,
	Victory
};

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
	
	UFUNCTION(BlueprintCallable)
	void PauseGame();
};
