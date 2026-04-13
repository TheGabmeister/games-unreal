#pragma once

#include "CoreMinimal.h"
#include "QuakeTrigger.h"
#include "QuakeTrigger_Exit.generated.h"

/**
 * Phase 8 level-exit trigger per SPEC section 5.6. On activation, opens
 * the map named NextMapName via UGameplayStatics::OpenLevel. Fires the
 * base Targets chain first so any on-exit bookkeeping triggers (message,
 * relay, secret-wrap) run before the map transition.
 *
 * Phase 9 replaces the direct OpenLevel call with a GameMode-mediated
 * level-clear flow that checks win conditions first (all-kills-required,
 * boss-dead, etc.). Until then this is the minimal "walk into the trigger
 * and load the next map" stub the Phase 8 sandbox needs.
 */
UCLASS()
class QUAKE_API AQuakeTrigger_Exit : public AQuakeTrigger
{
	GENERATED_BODY()

public:
	/** Map to open on activation. Empty = log a warning and do nothing. */
	UPROPERTY(EditInstanceOnly, Category = "Trigger|Exit")
	FName NextMapName;

	virtual void Activate(AActor* InInstigator) override;
};
