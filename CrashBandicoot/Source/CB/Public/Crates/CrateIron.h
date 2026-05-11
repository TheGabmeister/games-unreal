// CrateIron.h
#pragma once
#include "CoreMinimal.h"
#include "Crates/CrateBase.h"
#include "CrateIron.generated.h"

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ACrateIron : public ACrateBase
{
    GENERATED_BODY()
public:
    ACrateIron();

    virtual void OnSpinHit(ACBPlayerCharacter* Player) override {}
    virtual void OnJumpHit(ACBPlayerCharacter* Player) override {}
    virtual void OnExplosionHit(FVector Origin, float Radius) override {}
};
