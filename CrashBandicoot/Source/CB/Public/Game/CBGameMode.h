#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CBGameMode.generated.h"

UCLASS(Abstract, Blueprintable, meta = (PrioritizeCategories = "CB"))
class CB_API ACBGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACBGameMode();

    void PlayerDied();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float RestartDelay = 3.0f;

private:
    void HandleGameOver();
    void ResetCurrentLevel();

    FTimerHandle TimerHandle_Restart;
};
