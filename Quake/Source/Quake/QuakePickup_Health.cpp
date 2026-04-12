#include "QuakePickup_Health.h"

#include "QuakeCharacter.h"

bool AQuakePickup_Health::CanBeConsumedBy(AQuakeCharacter* Character) const
{
	if (!Character || Character->IsDead())
	{
		return false;
	}
	if (bIsOvercharge)
	{
		// Megahealth always consumed — the overcharge rule means it can
		// push HP past Max, so "already at max" is no longer a reason to
		// skip. Only refuse if already at the overcharge cap.
		return Character->GetHealth() < AQuakeCharacter::GetOverchargeCap();
	}
	// Normal health: SPEC 4.1 — only consumed if HP < Max.
	return Character->GetHealth() < Character->GetMaxHealth();
}

void AQuakePickup_Health::ApplyPickupEffectTo(AQuakeCharacter* Character)
{
	if (!Character)
	{
		return;
	}
	Character->GiveHealth(HealthAmount, bIsOvercharge);
}
