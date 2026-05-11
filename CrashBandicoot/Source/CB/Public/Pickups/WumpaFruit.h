// WumpaFruit.h
#pragma once
#include "CoreMinimal.h"
#include "Pickups/PickupBase.h"
#include "WumpaFruit.generated.h"

UCLASS()
class CB_API AWumpaFruit : public APickupBase
{
    GENERATED_BODY()
protected:
    virtual void OnPickedUp(ACBPlayerCharacter* Player) override;
};
