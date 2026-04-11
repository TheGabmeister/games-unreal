#pragma once

#include "CoreMinimal.h"
#include "QuakeEnemyBase.h"
#include "QuakeEnemy_Grunt.generated.h"

/**
 * Phase 3 Grunt pawn. A hitscan-rifle soldier with the stats from SPEC
 * section 3.1:
 *   HP=30, Speed=300, Sight=2000, Hearing=1500,
 *   AttackRange=1500, AttackDamage=4, AttackCooldown=1.5s.
 *
 * Per the project convention, all stats are set as UPROPERTY defaults in
 * the constructor. The thin BP_Enemy_Grunt subclass only fills in asset
 * slots (body / head / rifle primitive meshes, material instance, pain /
 * death sound slots, drop table entries for Phase 7). Zero nodes in the
 * BP event graph.
 *
 * The Grunt's attack is hitscan with UQuakeDamageType_Bullet, so it
 * overrides FireAtTarget to trace on the Weapon channel and call
 * ApplyPointDamage. Everything else (perception, FSM, MoveTo) lives on the
 * base class / controller.
 */
UCLASS()
class QUAKE_API AQuakeEnemy_Grunt : public AQuakeEnemyBase
{
	GENERATED_BODY()

public:
	AQuakeEnemy_Grunt();

	virtual void FireAtTarget(AActor* Target) override;
};
