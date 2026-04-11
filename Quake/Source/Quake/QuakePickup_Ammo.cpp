#include "QuakePickup_Ammo.h"

#include "QuakeCharacter.h"

AQuakePickup_Ammo::AQuakePickup_Ammo()
{
	// Defaults to a small shell pack. BP subclasses set AmmoType + AmmoAmount.
}

void AQuakePickup_Ammo::ApplyPickupEffectTo(AQuakeCharacter* Character)
{
	if (!Character)
	{
		return;
	}
	Character->GiveAmmo(AmmoType, AmmoAmount);
}
