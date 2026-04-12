#pragma once

#include "CoreMinimal.h"
#include "QuakeEnemyBase.h"
#include "QuakeEnemy_Ogre.generated.h"

class AQuakeProjectile;

/**
 * Phase 7 Ogre pawn. SPEC section 3.1:
 *   HP=200, Speed=250, Sight=2500, Hearing=2000,
 *   Melee range=96, Melee damage=20, Cooldown=2.0s,
 *   Grenade range=1500, Grenade splash=40, Proj speed=600 (arc).
 *
 * Dual-mode attack: lobs grenades at range, switches to melee (chainsaw)
 * in close. The controller decides which to call; the pawn exposes both.
 */
UCLASS()
class QUAKE_API AQuakeEnemy_Ogre : public AQuakeEnemyBase
{
	GENERATED_BODY()

public:
	AQuakeEnemy_Ogre();

	/** Melee (chainsaw) attack — short-range trace. */
	virtual void FireAtTarget(AActor* Target) override;

	/** Ranged grenade lob. Called by the Ogre controller when out of melee range. */
	void FireGrenadeAtTarget(AActor* Target);

	// --- Melee stats ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Ogre|Melee", meta = (ClampMin = "0.0"))
	float MeleeRange = 96.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Ogre|Melee", meta = (ClampMin = "0.0"))
	float MeleeDamage = 20.f;

	// --- Grenade stats ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Ogre|Grenade")
	TSubclassOf<AQuakeProjectile> GrenadeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Ogre|Grenade", meta = (ClampMin = "0.0"))
	float GrenadeSpawnForwardOffset = 60.f;
};
