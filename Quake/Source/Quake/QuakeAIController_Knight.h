#pragma once

#include "CoreMinimal.h"
#include "QuakeEnemyAIController.h"
#include "QuakeAIController_Knight.generated.h"

/**
 * Knight AI controller. The Knight charges directly at the target and
 * swings melee — the base FSM already handles this (Chase to
 * AttackRange, fire on cooldown). No Knight-specific overrides needed.
 */
UCLASS()
class QUAKE_API AQuakeAIController_Knight : public AQuakeEnemyAIController
{
	GENERATED_BODY()

public:
	AQuakeAIController_Knight();
};
