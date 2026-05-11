#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BlinkComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CB_API UBlinkComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UBlinkComponent();

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float BlinkInterval = 0.1f;

    UFUNCTION(BlueprintCallable, Category = "CB")
    void StartBlinking();

    UFUNCTION(BlueprintCallable, Category = "CB")
    void StopBlinking();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CB")
    bool IsBlinking() const { return bIsBlinking; }

private:
    void ToggleVisibility();

    FTimerHandle TimerHandle_Blink;
    bool bIsBlinking = false;
};
