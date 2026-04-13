#pragma once

#include "CoreMinimal.h"
#include "QuakeTrigger.h"
#include "QuakeTrigger_Exit.generated.h"

/**
 * Level-exit trigger per SPEC sections 5.6 and 5.9. On activation:
 *   1. If bGatedByClearCondition, consult AQuakeGameMode::IsLevelCleared
 *      and bail (with a HUD message) if the level isn't clear.
 *   2. Fire the base Targets chain so any on-exit bookkeeping runs before
 *      the map transition wipes everything.
 *   3. Show the level-end stats screen for StatsDisplaySeconds.
 *   4. Schedule OpenLevel(NextMapName) via a timer at the same duration.
 *
 * Phase 9 wires (1)-(4). Phase 11 adds the save-game auto-save between
 * (2) and (3). Phase 10 adds the post-level loadout rollover to the hub.
 */
UCLASS()
class QUAKE_API AQuakeTrigger_Exit : public AQuakeTrigger
{
	GENERATED_BODY()

public:
	AQuakeTrigger_Exit();

	/** Map to open on activation. Empty = log a warning and do nothing. */
	UPROPERTY(EditInstanceOnly, Category = "Trigger|Exit")
	FName NextMapName;

	/**
	 * If true, the exit refuses to fire while
	 * AQuakeGameMode::IsLevelCleared returns false. A locked-exit message
	 * is shown on the HUD instead. If false, the exit fires immediately —
	 * useful for hub exits and escape sequences.
	 */
	UPROPERTY(EditInstanceOnly, Category = "Trigger|Exit")
	bool bGatedByClearCondition = true;

	/**
	 * Seconds the end-of-level stats screen displays before OpenLevel
	 * fires. SPEC section 11.5 Phase 9 "shown for 5 s before transition".
	 */
	UPROPERTY(EditInstanceOnly, Category = "Trigger|Exit", meta = (ClampMin = "0.1"))
	float StatsDisplaySeconds = 5.f;

	virtual void Activate(AActor* InInstigator) override;

private:
	UFUNCTION()
	void OnStatsScreenTimeout();

	FTimerHandle TransitionTimer;

	/** Prevents a second overlap from firing the transition a second time. */
	bool bTransitionInFlight = false;
};
