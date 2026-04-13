#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuakeHazardVolume.generated.h"

class UBoxComponent;
class UDamageType;

/**
 * Phase 8 hazard volume per SPEC section 5.2. Covers lava (30 dmg / 1.0 s
 * + 200 u entry knockback) and slime (4 dmg / 1.0 s, no knockback). The
 * specific hazard is selected by assigning DamageTypeClass and tuning
 * DamagePerTick / TickRate / EntryKnockbackStrength in the BP subclass
 * (BP_HazardVolume_Lava / BP_HazardVolume_Slime).
 *
 * Damage flows through UE's TakeDamage pipeline using the configured damage
 * type (UQuakeDamageType_Lava / _Slime) — the pipeline's bSuppressesPain
 * flag is what prevents the damage flash / flinch every tick for lava.
 *
 * Per-pawn FTimerHandle map so each occupant ticks on its own schedule from
 * the moment it entered. BeginOverlap: apply one-shot entry knockback (if
 * configured) and start the damage timer. EndOverlap: clear the timer.
 */
UCLASS()
class QUAKE_API AQuakeHazardVolume : public AActor
{
	GENERATED_BODY()

public:
	AQuakeHazardVolume();

	/** Box volume root — overlap-only, configured in the BP subclass to
	 *  match the visible lava / slime geometry. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hazard")
	TObjectPtr<UBoxComponent> VolumeBox;

	/** Damage type fired on each tick. Set in the BP subclass (Lava / Slime). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hazard")
	TSubclassOf<UDamageType> DamageTypeClass;

	/** Damage applied per tick to every overlapping pawn. SPEC: 30 (lava), 4 (slime). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hazard", meta = (ClampMin = "0.0"))
	float DamagePerTick = 30.f;

	/** Seconds between damage ticks. SPEC: 1.0 for both lava and slime. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hazard", meta = (ClampMin = "0.01"))
	float TickRate = 1.f;

	/** Horizontal launch velocity applied once when a pawn enters the volume.
	 *  SPEC: 200 for lava, 0 for slime. Applied as LaunchCharacter with
	 *  bXYOverride = true, direction = (pawn - volume center) normalized. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hazard", meta = (ClampMin = "0.0"))
	float EntryKnockbackStrength = 200.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Timer callback that applies one tick of DamagePerTick to the given pawn. */
	void ApplyTickDamage(TWeakObjectPtr<AActor> Pawn);

private:
	/** One repeating timer per currently-overlapping pawn. Pawn pointer is
	 *  the key — cleared in EndOverlap / EndPlay to avoid dangling timers. */
	TMap<TWeakObjectPtr<AActor>, FTimerHandle> ActiveTimers;
};
