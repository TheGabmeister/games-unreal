// AkuAkuPickup.cpp
#include "Pickups/AkuAkuPickup.h"
#include "Player/CBPlayerCharacter.h"

void AAkuAkuPickup::OnPickedUp(ACBPlayerCharacter* Player)
{
    if (Player)
    {
        Player->AddMask();
    }
}
