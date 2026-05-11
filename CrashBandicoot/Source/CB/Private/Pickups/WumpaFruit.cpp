// WumpaFruit.cpp
#include "Pickups/WumpaFruit.h"
#include "Game/CBGameInstance.h"
#include "Kismet/GameplayStatics.h"

void AWumpaFruit::OnPickedUp(ACBPlayerCharacter* Player)
{
    if (UCBGameInstance* GI = Cast<UCBGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        GI->AddWumpa(1);
    }
}
