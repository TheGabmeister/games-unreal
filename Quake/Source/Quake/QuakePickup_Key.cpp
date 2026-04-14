#include "QuakePickup_Key.h"

#include "QuakeCharacter.h"

bool AQuakePickup_Key::CanBeConsumedBy(AQuakeCharacter* Character) const
{
	if (!Character || Character->IsDead())
	{
		return false;
	}
	if (KeyColor == EQuakeKeyColor::None)
	{
		return false;
	}
	// SPEC 4.4: re-pickup is a no-op. Leave the key in the world.
	return !Character->HasKey(KeyColor);
}

void AQuakePickup_Key::ApplyPickupEffectTo(AQuakeCharacter* Character)
{
	if (!Character)
	{
		return;
	}
	Character->GiveKey(KeyColor);
}
