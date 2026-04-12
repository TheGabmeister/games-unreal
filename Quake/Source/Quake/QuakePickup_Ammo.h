#pragma once

#include "CoreMinimal.h"
#include "QuakeAmmoType.h"
#include "QuakePickupBase.h"
#include "QuakePickup_Ammo.generated.h"

/**
 * Phase 4 ammo pickup. SPEC section 2.1 pickup amounts:
 *   - Shells:  Small 20,  Large 40
 *   - Nails:   Small 25,  Large 50
 *   - Rockets: Small 5,   Large 10
 *   - Cells:   Small 6,   Large 12
 *
 * Each BP variant (BP_Pickup_AmmoShells, etc.) sets AmmoType + AmmoAmount
 * in editor defaults. Ammo pickups are always consumed — even at cap —
 * matching original Quake (the excess is wasted). Can be revisited if
 * the "no waste" behavior is ever desired.
 */
UCLASS()
class QUAKE_API AQuakePickup_Ammo : public AQuakePickupBase
{
	GENERATED_BODY()

public:
	/** Ammo type this pickup grants. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Ammo")
	EQuakeAmmoType AmmoType = EQuakeAmmoType::Shells;

	/** Amount granted on pickup. Defaults to a small shell pack. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Ammo", meta = (ClampMin = "0"))
	int32 AmmoAmount = 20;

	virtual void ApplyPickupEffectTo(AQuakeCharacter* Character) override;
};
