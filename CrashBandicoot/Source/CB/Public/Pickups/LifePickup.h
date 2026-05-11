// LifePickup.h
#pragma once
#include "CoreMinimal.h"
#include "Pickups/PickupBase.h"
#include "LifePickup.generated.h"

UCLASS()
class CB_API ALifePickup : public APickupBase
{
    GENERATED_BODY()
protected:
    virtual void OnPickedUp(ACBPlayerCharacter* Player) override;
};
