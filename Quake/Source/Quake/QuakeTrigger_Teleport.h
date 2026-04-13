#pragma once

#include "CoreMinimal.h"
#include "QuakeTrigger.h"
#include "QuakeTrigger_Teleport.generated.h"

/**
 * Phase 8 teleport trigger per SPEC section 5.6. Teleports the overlapping
 * pawn to Destination's transform. Velocity magnitude is preserved but
 * rotated to the destination's yaw, so momentum carries through. Then
 * fires Super::Activate to chain any base Targets.
 *
 * Destination is typed AActor* so any actor works — typically an engine
 * ATargetPoint placed in the level.
 */
UCLASS()
class QUAKE_API AQuakeTrigger_Teleport : public AQuakeTrigger
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, Category = "Trigger|Teleport")
	TObjectPtr<AActor> Destination;

	virtual void Activate(AActor* InInstigator) override;
};
