#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "QuakeDamageType.generated.h"

/**
 * Abstract base for every Quake damage type. Owns ALL Quake-specific
 * per-source metadata as UPROPERTY fields; leaf subclasses (Melee, Bullet,
 * Nail, Explosive, Lightning, Lava, Slime, Drown, Telefrag) add no new
 * properties and only override defaults in their constructor.
 *
 * This consolidation is what lets TakeDamage remain free of per-class
 * branching: the override does ONE cast to UQuakeDamageType (via
 * DamageTypeClass->GetDefaultObject()) and reads the fields uniformly.
 * Never branch on leaf class identity. See SPEC section 1.5.
 *
 * Engine-provided fields from UDamageType are reused where they fit
 * (bCausedByWorld, bScaleMomentumByMass, DamageImpulse, DamageFalloff) —
 * do NOT shadow them here.
 *
 * Phase 0 scope: the abstract base only. No leaf subclasses yet — those
 * come online in Phase 2 (Melee) and later phases.
 */
UCLASS(Abstract)
class QUAKE_API UQuakeDamageType : public UDamageType
{
	GENERATED_BODY()

public:
	/** Armor does not absorb this damage (e.g. Lightning, Drown). */
	UPROPERTY(EditDefaultsOnly, Category = "Quake Damage")
	bool bIgnoresArmor = false;

	/** Pain reaction suppressed on the target (e.g. Lava ticks, Telefrag). */
	UPROPERTY(EditDefaultsOnly, Category = "Quake Damage")
	bool bSuppressesPain = false;

	/** Biosuit powerup does NOT protect against this damage type (e.g. Drown). */
	UPROPERTY(EditDefaultsOnly, Category = "Quake Damage")
	bool bBypassesBiosuit = false;

	/** Whether self-damage applies when the instigator is also the victim. */
	UPROPERTY(EditDefaultsOnly, Category = "Quake Damage")
	bool bSelfDamage = true;

	/** Scalar applied to damage when hitting the instigator (0.5 for explosives, 1.0 default). */
	UPROPERTY(EditDefaultsOnly, Category = "Quake Damage")
	float SelfDamageScale = 1.0f;

	/** Scalar on the knockback impulse magnitude (4.0 for explosives to enable rocket jumps). */
	UPROPERTY(EditDefaultsOnly, Category = "Quake Damage")
	float KnockbackScale = 1.0f;
};
