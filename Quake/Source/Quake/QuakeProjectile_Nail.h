#pragma once

#include "CoreMinimal.h"
#include "QuakeProjectile.h"
#include "QuakeProjectile_Nail.generated.h"

/**
 * Phase 6 nail projectile. SPEC section 2.0 weapon table row 4:
 *   - Speed:  1500 u/s straight (no gravity)
 *   - Damage: 9 per hit
 *   - Damage type: UQuakeDamageType_Nail
 *
 * Unlike the Rocket, nails apply a single point-damage event on direct
 * impact and have no splash radius. Knockback is naturally proportional to
 * damage via the base TakeDamage formula (ScaledDamage * 30 * 1.0 = 270
 * magnitude per nail, which stacks across sustained fire).
 *
 * Per the project convention, all stats are UPROPERTY defaults set in the
 * C++ constructor. The thin BP_Projectile_Nail subclass only fills in the
 * mesh + material asset slots — zero nodes in the BP event graph.
 */
UCLASS()
class QUAKE_API AQuakeProjectile_Nail : public AQuakeProjectile
{
	GENERATED_BODY()

public:
	AQuakeProjectile_Nail();

	/** Damage applied to a direct-hit target. SPEC: 9. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Nail", meta = (ClampMin = "0.0"))
	float BaseDamage = 9.f;

protected:
	virtual void HandleImpact(const FHitResult& Hit, AActor* OtherActor) override;
};
