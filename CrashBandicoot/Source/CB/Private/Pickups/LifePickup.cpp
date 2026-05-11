// LifePickup.cpp
#include "Pickups/LifePickup.h"
#include "Game/CBGameInstance.h"
#include "Kismet/GameplayStatics.h"

void ALifePickup::OnPickedUp(ACBPlayerCharacter* Player)
{
    if (UCBGameInstance* GI = Cast<UCBGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        GI->AddLife();
    }
}
