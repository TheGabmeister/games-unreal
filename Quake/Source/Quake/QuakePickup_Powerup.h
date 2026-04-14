#pragma once

#include "CoreMinimal.h"
#include "QuakePickupBase.h"
#include "QuakePowerup.h"
#include "QuakePickup_Powerup.generated.h"

/**
 * Phase 10 powerup pickup per SPEC 4.3. Single concrete class; BP variants
 * (BP_Pickup_Quad, future Pentagram / Ring / Biosuit) differ only in the
 * Type + Duration + mesh/material/tint asset slots.
 *
 * SPEC 4.3: "Picking up a powerup is always consumed — even at cap the
 * pickup is removed. Refresh logic lives in PlayerState::GivePowerup
 * (additive, capped at 60 s)." So CanBeConsumedBy stays as the base's
 * "yes, always" default — the consume-vs-refresh decision is PlayerState's.
 */
UCLASS()
class QUAKE_API AQuakePickup_Powerup : public AQuakePickupBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Powerup")
	EQuakePowerup Type = EQuakePowerup::Quad;

	/** Seconds granted by this pickup. SPEC 4.3 defaults to 30 for every v1 powerup. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Powerup", meta = (ClampMin = "0.0"))
	float Duration = 30.f;

	virtual void ApplyPickupEffectTo(AQuakeCharacter* Character) override;
};
