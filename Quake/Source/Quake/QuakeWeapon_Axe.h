#pragma once

#include "CoreMinimal.h"
#include "QuakeWeaponBase.h"
#include "QuakeWeapon_Axe.generated.h"

/**
 * Phase 2 melee weapon. Single short-range trace per swing on the Weapon
 * collision channel, dealing flat damage with UQuakeDamageType_Melee.
 *
 * SPEC section 2.0 weapon table:
 *   - Damage:  20
 *   - Range:   64 units
 *   - RoF:     2 swings/sec  (set on the base class)
 *   - Spread:  none
 *   - Knockback: 0
 *
 * Per the project convention, all stats live as UPROPERTY defaults set in
 * the constructor. The thin BP_Weapon_Axe subclass only fills in the
 * viewmodel mesh slot — zero nodes in the BP event graph.
 */
UCLASS()
class QUAKE_API AQuakeWeapon_Axe : public AQuakeWeaponBase
{
	GENERATED_BODY()

public:
	AQuakeWeapon_Axe();

	/** Damage applied per successful melee trace hit. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Axe", meta = (ClampMin = "0.0"))
	float Damage = 20.f;

	/** Maximum trace length from the firer's eyes, in unreal units. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Axe", meta = (ClampMin = "0.0"))
	float Range = 64.f;

protected:
	virtual void Fire(AActor* InInstigator) override;
};
