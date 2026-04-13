#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuakeActivatable.h"
#include "QuakeTrigger.generated.h"

class UBoxComponent;

/**
 * Phase 8 trigger base per SPEC section 5.6. Abstract. Every subclass
 * implements IQuakeActivatable so triggers can chain.
 *
 * Dual-input design: the same trigger actor can fire either by pawn
 * overlap (entry trigger) OR by Activate(Instigator) call (relay). The
 * overlap binding is set up here in the base so subclasses only need to
 * override Activate and (optionally) add type-specific fields. Authoring
 * decision: leave TriggerVolume's collision on for entry triggers, turn
 * it off for pure-relay triggers.
 *
 * Targets are stored as TArray<TObjectPtr<AActor>> and filled per-instance
 * in the editor — no string-name lookups, matching SPEC 5.5's actor-picker
 * model. Entries that don't implement IQuakeActivatable log a warning
 * (authoring error, not a crash).
 */
UCLASS(Abstract)
class QUAKE_API AQuakeTrigger : public AActor, public IQuakeActivatable
{
	GENERATED_BODY()

public:
	AQuakeTrigger();

	/** Overlap volume. Default collision overlaps Pawn; disable to run as
	 *  a pure relay (fires only when another trigger Activate()s it). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	TObjectPtr<UBoxComponent> TriggerVolume;

	/** Generic targets fired when this trigger activates. Subclasses MAY
	 *  also declare typed reference fields for picker filtering. */
	UPROPERTY(EditInstanceOnly, Category = "Trigger")
	TArray<TObjectPtr<AActor>> Targets;

	virtual void Activate(AActor* InInstigator) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Fire every entry in Targets via IQuakeActivatable, logging authoring errors. */
	void FireTargets(AActor* InInstigator);
};
