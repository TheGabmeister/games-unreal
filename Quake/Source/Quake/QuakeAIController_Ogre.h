#pragma once

#include "CoreMinimal.h"
#include "QuakeEnemyAIController.h"
#include "QuakeAIController_Ogre.generated.h"

/**
 * Ogre AI controller. Overrides the Attack state to choose between melee
 * (chainsaw) when the target is within MeleeRange, and grenade lob when
 * the target is farther away. The base FSM handles everything else.
 *
 * SPEC 3.1: Melee range 96 u, Grenade range 1500 u.
 */
UCLASS()
class QUAKE_API AQuakeAIController_Ogre : public AQuakeEnemyAIController
{
	GENERATED_BODY()

public:
	AQuakeAIController_Ogre();

	/** Threshold below which the Ogre uses melee instead of grenades. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Ogre", meta = (ClampMin = "0.0"))
	float MeleeThreshold = 96.f;

protected:
	virtual void Tick(float DeltaTime) override;
};
