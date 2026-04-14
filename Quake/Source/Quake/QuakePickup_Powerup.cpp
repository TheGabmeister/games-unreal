#include "QuakePickup_Powerup.h"

#include "QuakeCharacter.h"
#include "QuakePlayerState.h"

#include "GameFramework/Controller.h"

void AQuakePickup_Powerup::ApplyPickupEffectTo(AQuakeCharacter* Character)
{
	if (!Character)
	{
		return;
	}
	AController* C = Character->GetController();
	AQuakePlayerState* PS = C ? C->GetPlayerState<AQuakePlayerState>() : nullptr;
	if (!PS)
	{
		return;
	}
	PS->GivePowerup(Type, Duration);
}
