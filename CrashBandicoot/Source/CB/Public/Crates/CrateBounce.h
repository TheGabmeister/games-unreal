// CrateBounce.h
#pragma once
#include "CoreMinimal.h"
#include "Crates/CrateBase.h"
#include "CrateBounce.generated.h"

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ACrateBounce : public ACrateBase
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, Category = "CB")
    int32 MaxBounces = 10;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TObjectPtr<USoundBase> BounceSound;

    virtual void OnSpinHit(ACBPlayerCharacter* Player) override;
    virtual void OnJumpHit(ACBPlayerCharacter* Player) override;

protected:
    virtual void SpawnContents() override;
    void SpawnSingleWumpa();

    int32 BounceCount = 0;
    bool bHitThisFrame = false;
};
