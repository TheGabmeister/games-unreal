// AkuAkuPickup.h
#pragma once
#include "CoreMinimal.h"
#include "Pickups/PickupBase.h"
#include "AkuAkuPickup.generated.h"

UCLASS()
class CB_API AAkuAkuPickup : public APickupBase
{
    GENERATED_BODY()
protected:
    virtual void OnPickedUp(ACBPlayerCharacter* Player) override;
};
