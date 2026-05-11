// CrateTNT.h
#pragma once
#include "CoreMinimal.h"
#include "Crates/CrateBase.h"
#include "CrateTNT.generated.h"

class UTextRenderComponent;

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ACrateTNT : public ACrateBase
{
    GENERATED_BODY()
public:
    ACrateTNT();

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float BlastRadius = 250.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float ChainDetonationDelay = 0.05f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TObjectPtr<USoundBase> TickSound;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TObjectPtr<USoundBase> ExplosionSound;

    virtual void OnSpinHit(ACBPlayerCharacter* Player) override;
    virtual void OnJumpHit(ACBPlayerCharacter* Player) override;
    virtual void OnExplosionHit(FVector Origin, float Radius) override;

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UTextRenderComponent> CountdownText;

    void StartCountdown();
    void TickCountdown();
    void Detonate();

    FTimerHandle TimerHandle_Countdown;
    int32 CountdownValue = 3;
    bool bCountdownStarted = false;
    bool bDetonated = false;
};
