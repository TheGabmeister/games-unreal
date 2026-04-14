#pragma once

#include "CoreMinimal.h"
#include "QuakeAmmoType.h"
#include "QuakePickupBase.h"
#include "QuakeWeaponBase.h"
#include "QuakePickup_Weapon.generated.h"

/**
 * Phase 10 weapon pickup per SPEC 2.2 "first pickup grants weapon + ammo
 * and auto-switches; subsequent pickup grants only ammo". BP variants
 * (BP_Pickup_Weapon_Shotgun, etc.) fill WeaponClass + SlotNumber + AmmoType
 * + AmmoAmount — ammo values are on the pickup (matching Quake: the weapon
 * itself has no intrinsic "starting ammo").
 *
 * Note: TSubclassOf<AQuakeWeaponBase> requires the full weapon header in
 * this .h, not a forward decl — CLAUDE.md "TSubclassOf in .cpp requires
 * the full T header" applies to the .h too when the TSubclassOf lives in
 * a UPROPERTY, because UHT-generated code needs T::StaticClass() visible.
 */
UCLASS()
class QUAKE_API AQuakePickup_Weapon : public AQuakePickupBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Weapon")
	TSubclassOf<AQuakeWeaponBase> WeaponClass;

	/** SPEC 2.0 weapon number (1..8). Maps to OwnedWeaponClasses[Slot-1]. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Weapon", meta = (ClampMin = "1", ClampMax = "8"))
	int32 SlotNumberOneBased = 2;

	/** Ammo type and amount granted alongside the weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Weapon")
	EQuakeAmmoType AmmoType = EQuakeAmmoType::Shells;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Weapon", meta = (ClampMin = "0"))
	int32 AmmoAmount = 10;

	virtual void ApplyPickupEffectTo(AQuakeCharacter* Character) override;
	virtual EQuakeSoundEvent GetPickupSoundEvent() const override { return EQuakeSoundEvent::PickupWeapon; }
};
