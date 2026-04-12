#include "QuakePickup_Ammo.h"

#include "QuakeCharacter.h"

void AQuakePickup_Ammo::ApplyPickupEffectTo(AQuakeCharacter* Character)
{
	if (!Character)
	{
		return;
	}
	Character->GiveAmmo(AmmoType, AmmoAmount);
}
