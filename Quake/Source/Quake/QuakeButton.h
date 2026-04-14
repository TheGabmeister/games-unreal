#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuakeSaveable.h"
#include "QuakeButton.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EQuakeButtonActivation : uint8
{
	/** Fires when any ACharacter begin-overlaps the box. */
	Touch  UMETA(DisplayName = "Touch"),
	/** Fires when a weapon-channel hit registers damage on the box. */
	Shoot  UMETA(DisplayName = "Shoot")
};

/**
 * Phase 8 button per SPEC section 5.5. Holds a TArray<TObjectPtr<AActor>>
 * of targets filled per-instance via the editor's actor picker; on
 * activation, iterates the list, casts each to IQuakeActivatable, and
 * calls Activate(Instigator). Non-IQuakeActivatable entries log a
 * warning (authoring error, not a crash).
 *
 * Activation model is `EQuakeButtonActivation::Touch` or `Shoot`. The
 * button does NOT implement IQuakeActivatable — buttons are input
 * sources, not chainable links. Use AQuakeTrigger_Relay for chaining.
 *
 * One-shot (Cooldown <= 0) or reusable (Cooldown > 0): one-shot buttons
 * disable their collision after the first fire. Reusable buttons re-arm
 * after `Cooldown` seconds.
 */
UCLASS()
class QUAKE_API AQuakeButton : public AActor, public IQuakeSaveable
{
	GENERATED_BODY()

public:
	AQuakeButton();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quake|Button")
	EQuakeButtonActivation ActivationMode = EQuakeButtonActivation::Touch;

	/** Seconds until the button re-arms after firing. 0 = one-shot. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quake|Button", meta = (ClampMin = "0.0"))
	float Cooldown = 0.f;

	/** Actors to fire on activation. Each must implement IQuakeActivatable. */
	UPROPERTY(EditInstanceOnly, Category = "Quake|Button")
	TArray<TObjectPtr<AActor>> Targets;

	// --- Components ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quake|Button")
	TObjectPtr<UBoxComponent> Collider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quake|Button")
	TObjectPtr<UStaticMeshComponent> Mesh;

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;

	// IQuakeSaveable
	virtual void SaveState(FActorSaveRecord& OutRecord) override;
	virtual void LoadState(const FActorSaveRecord& InRecord) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnColliderBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Fire all Targets. Handles authoring errors (null / non-IQuakeActivatable entries). */
	void Fire(AActor* InInstigator);

	UFUNCTION()
	void ReArm();

private:
	UPROPERTY(meta = (SaveGame))
	bool bArmed = true;

	FTimerHandle CooldownHandle;
};
