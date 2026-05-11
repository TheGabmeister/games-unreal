#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CBPlayerCharacter.generated.h"

class AAkuAkuMaskActor;
class UBlinkComponent;
class UBoxComponent;
class UInputAction;
class UInputMappingContext;
class UAnimMontage;
class USphereComponent;

UENUM(BlueprintType)
enum class EAkuAkuState : uint8
{
	None,
	OneHit,
	TwoHits,
	Invincible
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAkuAkuStateChanged, EAkuAkuState, NewState);

UCLASS(Abstract, Blueprintable)
class CB_API ACBPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACBPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	// --- Spin ---

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CB")
	bool IsSpinning() const { return bIsSpinning; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CB")
	int32 GetSpinCharges() const { return SpinCharges; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CB")
	bool IsJumpInputActive() const { return bPressedJump; }

	// --- Damage System ---

	UFUNCTION(BlueprintCallable, Category = "CB")
	void OnHit(AActor* Source);

	UFUNCTION(BlueprintCallable, Category = "CB")
	void InstantKill();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CB")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CB")
	bool IsInvincible() const { return AkuAkuState == EAkuAkuState::Invincible; }

	// --- Aku Aku ---

	UFUNCTION(BlueprintCallable, Category = "CB")
	void AddMask();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CB")
	EAkuAkuState GetAkuAkuState() const { return AkuAkuState; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "CB")
	FOnPlayerDied OnPlayerDied;

	UPROPERTY(BlueprintAssignable, Category = "CB")
	FOnAkuAkuStateChanged OnAkuAkuStateChanged;

	// Default bounce velocity when stomping something (crates, enemies).
	// Individual actors can override this with their own value (e.g. arrow crates).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CB")
	float StompBounceVelocity = 800.0f;

	// --- ACharacter overrides ---

	virtual void StopJumping() override;
	virtual void CheckJumpInput(float DeltaTime) override;
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void Landed(const FHitResult& Hit) override;

	// --- Spin (called by controller) ---

	void DoSpin();
	void StopSpin();

	// --- Input handlers (called by controller) ---

	void Input_Move(const struct FInputActionValue& Value);
	void Input_Jump(const struct FInputActionValue& Value);
	void Input_StopJump(const struct FInputActionValue& Value);
	void Input_Spin(const struct FInputActionValue& Value);

	// --- Enemy Jump (existing functionality) ---

	UFUNCTION(BlueprintCallable)
	bool IsEnemyJumpValid(UBoxComponent* HurtBox);
	void JumpFromEnemyHurtBox();

protected:
	virtual void BeginPlay() override;

	// --- Spin System ---

	UPROPERTY(EditDefaultsOnly, Category = "CB", meta = (ClampMin = "1", UIMin = "1"))
	int32 MaxSpinCharges = 3;

	UPROPERTY(EditDefaultsOnly, Category = "CB", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float ChargeRegenInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "CB", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float SpinDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<UAnimMontage> SpinMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CB")
	TObjectPtr<USphereComponent> SpinAttackVolume;

	// --- Aku Aku ---

	UPROPERTY(BlueprintReadOnly, Category = "CB")
	EAkuAkuState AkuAkuState = EAkuAkuState::None;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TSubclassOf<AAkuAkuMaskActor> MaskActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<USoundBase> MaskPickupSound;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<USoundBase> MaskAbsorbSound;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<USoundBase> InvincibilitySound;

	UPROPERTY(EditDefaultsOnly, Category = "CB", meta = (ClampMin = "1.0"))
	float InvincibilityDuration = 20.0f;

	// --- Damage ---

	UPROPERTY(EditDefaultsOnly, Category = "CB", meta = (ClampMin = "0.0"))
	float PostHitInvulnerabilityDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float StompVelocityThreshold = -100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float KnockbackForce = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float MaskAbsorbKnockbackMultiplier = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<USoundBase> DeathSound;

	// --- Blink Component ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CB")
	TObjectPtr<UBlinkComponent> BlinkComponent;

	// --- Blueprint Events ---

	UFUNCTION(BlueprintImplementableEvent, Category = "CB")
	void OnSpinStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "CB")
	void OnSpinEnded();

	UFUNCTION(BlueprintImplementableEvent, Category = "CB")
	void OnEnemyJump();

	UFUNCTION(BlueprintImplementableEvent, Category = "CB")
	void OnCharacterDeath();

private:
	void RegenSpinCharge();
	void Die();
	void StartInvincibility();
	void EndInvincibility();
	void StartPostHitInvulnerability();
	void EndPostHitInvulnerability();

	void SpawnMaskActor();
	void DestroyMaskActor();
	void UpdateMaskVisual();

	UFUNCTION()
	void OnSpinOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	TObjectPtr<AAkuAkuMaskActor> MaskActor;

	FTimerHandle TimerHandle_SpinDuration;
	FTimerHandle TimerHandle_SpinChargeRegen;
	FTimerHandle TimerHandle_Invincibility;
	FTimerHandle TimerHandle_PostHitInvulnerability;

	int32 SpinCharges;
	bool bIsSpinning = false;
	bool bIsDead = false;
	bool bHitInvulnerable = false;
};
