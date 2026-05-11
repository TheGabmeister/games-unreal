// CrateIronArrow.h
#pragma once
#include "CoreMinimal.h"
#include "Crates/CrateBase.h"
#include "CrateIronArrow.generated.h"

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ACrateIronArrow : public ACrateBase
{
    GENERATED_BODY()
public:
    ACrateIronArrow();

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float LaunchVelocity = 1500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TObjectPtr<USoundBase> LaunchSound;

    virtual void OnSpinHit(ACBPlayerCharacter* Player) override {}
    virtual void OnJumpHit(ACBPlayerCharacter* Player) override;
    virtual void OnExplosionHit(FVector Origin, float Radius) override {}
};
