#pragma once

#include "CoreMinimal.h"
#include "QuakeEnemyBase.h"
#include "QuakeEnemy_Knight.generated.h"

/**
 * Phase 7 Knight pawn. A charging melee attacker per SPEC section 3.1:
 *   HP=75, Speed=400, Sight=2000, Hearing=1500,
 *   AttackRange=80, AttackDamage=10, AttackCooldown=1.0s.
 *
 * The Knight charges directly at the target and swings a melee attack
 * (short-range trace on the Weapon channel, UQuakeDamageType_Melee).
 * No ranged attack. FireAtTarget traces forward like the Axe but from
 * the Knight's eye viewpoint.
 */
UCLASS()
class QUAKE_API AQuakeEnemy_Knight : public AQuakeEnemyBase
{
	GENERATED_BODY()

public:
	AQuakeEnemy_Knight();

	virtual void FireAtTarget(AActor* Target) override;
};
