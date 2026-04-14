#pragma once

#include "CoreMinimal.h"
#include "QuakeKeyColor.h"
#include "QuakePickupBase.h"
#include "QuakePickup_Key.generated.h"

/**
 * Phase 10 key pickup per SPEC 4.4. BP variants (BP_Pickup_KeySilver /
 * KeyGold) differ only in the KeyColor + mesh/material asset slots.
 *
 * SPEC 4.4: "Picking up a key the player already holds is a no-op (no
 * message, no effect)." — CanBeConsumedBy rejects the pickup when the
 * player already has that color so the actor persists in the world rather
 * than quietly deleting itself.
 */
UCLASS()
class QUAKE_API AQuakePickup_Key : public AQuakePickupBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Key")
	EQuakeKeyColor KeyColor = EQuakeKeyColor::Silver;

	virtual bool CanBeConsumedBy(AQuakeCharacter* Character) const override;
	virtual void ApplyPickupEffectTo(AQuakeCharacter* Character) override;
};
