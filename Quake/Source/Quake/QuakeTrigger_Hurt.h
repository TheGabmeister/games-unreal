#pragma once

#include "CoreMinimal.h"
#include "QuakeTrigger.h"
#include "QuakeTrigger_Hurt.generated.h"

class UDamageType;

/**
 * Phase 8 hurt / kill trigger per SPEC section 5.6. Self-contained: ticks
 * DamagePerTick on every overlapping pawn via UQuakeDamageType_Telefrag.
 * Does NOT fire Targets — unlike other trigger subclasses, _Hurt is a
 * passive hazard, not a dispatcher. Used for kill floors, crusher pits,
 * and "you walked into the trap" zones.
 *
 * Per-pawn FTimerHandle map keyed by overlapping actor, same pattern as
 * AQuakeHazardVolume.
 */
UCLASS()
class QUAKE_API AQuakeTrigger_Hurt : public AQuakeTrigger
{
	GENERATED_BODY()

public:
	AQuakeTrigger_Hurt();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger|Hurt", meta = (ClampMin = "0.0"))
	float DamagePerTick = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger|Hurt", meta = (ClampMin = "0.01"))
	float TickRate = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trigger|Hurt")
	TSubclassOf<UDamageType> DamageTypeClass;

	virtual void Activate(AActor* InInstigator) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ApplyTickDamage(TWeakObjectPtr<AActor> Pawn);

private:
	TMap<TWeakObjectPtr<AActor>, FTimerHandle> ActiveTimers;
};
