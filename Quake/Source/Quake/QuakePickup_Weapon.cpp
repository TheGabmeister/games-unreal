#include "QuakePickup_Weapon.h"

#include "QuakeCharacter.h"

void AQuakePickup_Weapon::ApplyPickupEffectTo(AQuakeCharacter* Character)
{
	if (!Character || !WeaponClass)
	{
		return;
	}
	const int32 SlotIndex = SlotNumberOneBased - 1;
	// SPEC 2.2: always grant the ammo — even on a re-pickup — then decide
	// whether the weapon itself needs to spawn + auto-switch.
	if (AmmoType != EQuakeAmmoType::None && AmmoAmount > 0)
	{
		Character->GiveAmmo(AmmoType, AmmoAmount);
	}
	Character->GiveWeaponPickup(SlotIndex, WeaponClass);
}
