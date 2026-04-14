#pragma once

#include "CoreMinimal.h"
#include "QuakePickupBase.h"
#include "QuakePickup_Armor.generated.h"

UENUM(BlueprintType)
enum class EQuakeArmorTier : uint8
{
	Green  UMETA(DisplayName = "Green  (30% absorb, 100)"),
	Yellow UMETA(DisplayName = "Yellow (60% absorb, 150)"),
	Red    UMETA(DisplayName = "Red    (80% absorb, 200)")
};

/**
 * Phase 10 armor pickup per SPEC 4.2 + 1.2. Three BP variants
 * (BP_Pickup_Armor_Green / _Yellow / _Red) differ only in the Tier enum +
 * mesh/material asset slots. Values are derived from the tier in C++ —
 * keeping 100/30, 150/60, 200/80 in one place removes the risk of a BP
 * shipping with a mismatched Amount + Absorption.
 *
 * SPEC 1.2 pickup rule: "Armor does not stack — picking up a new armor
 * replaces the current one only if the new tier is higher OR current armor
 * is depleted below the new pickup's value." CanBeConsumedBy encodes that
 * rule so a fresh Green pickup is silently ignored when the player already
 * has 140 Yellow armor.
 */
UCLASS()
class QUAKE_API AQuakePickup_Armor : public AQuakePickupBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Armor")
	EQuakeArmorTier Tier = EQuakeArmorTier::Green;

	/** Armor points granted by this tier (100 / 150 / 200). */
	static float GetAmountForTier(EQuakeArmorTier Tier);

	/** Absorption ratio for this tier (0.3 / 0.6 / 0.8). */
	static float GetAbsorptionForTier(EQuakeArmorTier Tier);

	virtual bool CanBeConsumedBy(AQuakeCharacter* Character) const override;
	virtual void ApplyPickupEffectTo(AQuakeCharacter* Character) override;
};
