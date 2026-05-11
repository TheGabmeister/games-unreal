// CrateArrow.h
#pragma once
#include "CoreMinimal.h"
#include "Crates/CrateBase.h"
#include "CrateArrow.generated.h"

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ACrateArrow : public ACrateBase
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float LaunchVelocity = 3000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TObjectPtr<USoundBase> LaunchSound;

    virtual void OnJumpHit(ACBPlayerCharacter* Player) override;
    virtual void OnSpinHit(ACBPlayerCharacter* Player) override;
};
