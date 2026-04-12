#pragma once

#include "CoreMinimal.h"
#include "QuakePickupBase.h"
#include "QuakePickup_Health.generated.h"

/**
 * Phase 4 health pickup. SPEC section 4.1:
 *   - Small Health: 15 HP, only consumed if HP < Max.
 *   - Health Pack:  25 HP, only consumed if HP < Max.
 *   - Megahealth:  100 HP, overcharges up to 200 (see SPEC 1.2), always
 *                  consumed on touch.
 *
 * The three variants are Blueprints (BP_Pickup_Health15 /
 * BP_Pickup_Health25 / BP_Pickup_Megahealth) differing only in the
 * HealthAmount + bIsOvercharge UPROPERTY defaults + mesh/material asset
 * slots. Zero nodes in the BP event graph per project convention.
 */
UCLASS()
class QUAKE_API AQuakePickup_Health : public AQuakePickupBase
{
	GENERATED_BODY()

public:
	/** How much HP this pickup restores on consume. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Health", meta = (ClampMin = "0.0"))
	float HealthAmount = 25.f;

	/**
	 * Megahealth-style: always consumed, and the grant can push HP above
	 * MaxHealth up to the overcharge cap (SPEC 1.2: 200 HP, decays back to
	 * 100 at 1 HP/sec — the decay is Phase 10 polish, not Phase 4).
	 * Normal health pickups leave this false.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Health")
	bool bIsOvercharge = false;

	virtual bool CanBeConsumedBy(AQuakeCharacter* Character) const override;
	virtual void ApplyPickupEffectTo(AQuakeCharacter* Character) override;
};
