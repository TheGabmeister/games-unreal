#pragma once

#include "CoreMinimal.h"
#include "QuakeEnemyAIController.h"
#include "QuakeAIController_Grunt.generated.h"

/**
 * Grunt AI controller. Phase 3's only concrete enemy controller.
 *
 * The Grunt's behavior is the baseline FSM implemented in
 * AQuakeEnemyAIController — sight/hearing perception, Chase to AttackRange,
 * fire on cooldown, Pain flinch on hit. No Grunt-specific overrides are
 * needed (no leap, no grenade arc, no revive). This subclass exists
 * because the project convention is "one AIController subclass per enemy
 * type" (see [CLAUDE.md] "Architecture: AI Split"): even when the behavior
 * is identical to the base, the typed slot makes debugging and the AI
 * debugger's pawn → controller mapping unambiguous.
 *
 * SPEC section 3.1 Grunt stats (HP, Speed, Sight, Hearing, attack damage,
 * cooldown) live on AQuakeEnemy_Grunt as UPROPERTY defaults; the controller
 * reads them on OnPossess via the base class's perception-tuning step.
 */
UCLASS()
class QUAKE_API AQuakeAIController_Grunt : public AQuakeEnemyAIController
{
	GENERATED_BODY()

public:
	AQuakeAIController_Grunt();
};
