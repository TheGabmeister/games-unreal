#include "QuakePickup_Armor.h"

#include "QuakeCharacter.h"
#include "QuakeInventoryComponent.h"

float AQuakePickup_Armor::GetAmountForTier(EQuakeArmorTier InTier)
{
	// SPEC 4.2 table.
	switch (InTier)
	{
	case EQuakeArmorTier::Green:  return 100.f;
	case EQuakeArmorTier::Yellow: return 150.f;
	case EQuakeArmorTier::Red:    return 200.f;
	default:                      return 0.f;
	}
}

float AQuakePickup_Armor::GetAbsorptionForTier(EQuakeArmorTier InTier)
{
	switch (InTier)
	{
	case EQuakeArmorTier::Green:  return 0.3f;
	case EQuakeArmorTier::Yellow: return 0.6f;
	case EQuakeArmorTier::Red:    return 0.8f;
	default:                      return 0.f;
	}
}

bool AQuakePickup_Armor::CanBeConsumedBy(AQuakeCharacter* Character) const
{
	if (!Character || Character->IsDead())
	{
		return false;
	}
	const UQuakeInventoryComponent* Inv = Character->GetInventoryComponent();
	check(Inv);

	// SPEC 1.2: consume if the new tier absorbs more than current, OR if
	// current armor has drained below this pickup's value. The second clause
	// is the "Green re-up when you've used up your Yellow" path.
	const float NewAbsorb = GetAbsorptionForTier(Tier);
	const float NewAmount = GetAmountForTier(Tier);
	if (NewAbsorb > Inv->GetArmorAbsorption())
	{
		return true;
	}
	return Inv->GetArmor() < NewAmount;
}

void AQuakePickup_Armor::ApplyPickupEffectTo(AQuakeCharacter* Character)
{
	if (!Character)
	{
		return;
	}
	UQuakeInventoryComponent* Inv = Character->GetInventoryComponent();
	check(Inv);
	Inv->SetArmor(GetAmountForTier(Tier), GetAbsorptionForTier(Tier));
}
