
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/InteractionInterfaces.h"
#include "CBEnemyCharacterBase.generated.h"

class USplineComponent;
class UBoxComponent;
class ACBPlayerCharacter;
class ALaunchedEnemyProjectile;

USTRUCT(BlueprintType)
struct CB_API FEnemyInitializationArgs
{
	GENERATED_BODY();

public:
	TObjectPtr<USplineComponent> PatrolSpline;
	TObjectPtr<UBoxComponent> PatrolTriggerBox;
	TObjectPtr<UBoxComponent> AttackTriggerBox;

	UPROPERTY(EditInstanceOnly)
	bool LockXTransform = false;

	UPROPERTY(EditInstanceOnly)
	bool LockYTransform = false;

	UPROPERTY(EditInstanceOnly)
	bool LockZTransform = false;
};

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ACBEnemyCharacterBase : public ACharacter,
	public ISpinnable, public IStompable, public IExplodable
{
	GENERATED_BODY()

public:
	ACBEnemyCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CB")
	bool IsDead() const { return CurrentHitPoints <= 0; }

	UFUNCTION(BlueprintCallable, Category = "CB")
	virtual void HitCharacter();

	UFUNCTION(BlueprintCallable, Category = "CB")
	virtual void HitCharacterWithLaunchForce(const FVector& Force);

	UFUNCTION(BlueprintCallable, Category = "CB")
	void KillCharacter();

	// ISpinnable — default: kill enemy, launch as projectile
	virtual void OnSpinHit(ACBPlayerCharacter* Player) override;

	// IStompable — default: kill enemy, bounce player
	virtual void OnJumpHit(ACBPlayerCharacter* Player) override;

	// IExplodable — default: kill enemy
	virtual void OnExplosionHit(FVector Origin, float Radius) override;

	void Init(FEnemyInitializationArgs InitArgs);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CB")
	float GetMovementSpeedMultiplier();

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "10"), Category = "CB")
	int32 HitPoints = 1;

	UPROPERTY(BlueprintReadWrite, Category = "CB")
	int32 CurrentHitPoints = 0;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<USoundBase> DefeatSound;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float SpinLaunchForce = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	bool bSpinLaunchesAsProjectile = true;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = "0.0", UIMin = "0.0"), Category = "CB")
	float KnockbackForce = 500.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = "0.0", UIMin = "0.0"), Category = "CB")
	float AttackSpeedMultiplier = 4.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (ClampMin = "0.0", UIMin = "0.0"), Category = "CB")
	float HitStunDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "CB")
	TObjectPtr<USplineComponent> PatrolSpline = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "CB")
	TObjectPtr<UBoxComponent> PatrolTriggerVolume = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "CB")
	TObjectPtr<UBoxComponent> AttackTriggerVolume = nullptr;

	virtual void BeginPlay() override;

	virtual void HandleDefeat();
	void SpawnLaunchedProjectile(FVector LaunchDirection);

	UFUNCTION(BlueprintCallable, Category = "CB")
	FVector GetNextPatrolLocation();

	void IncrementPatrolPoint();

	UFUNCTION(BlueprintCallable, Category = "CB")
	void SetMovementSpeedMultiplier(float NewMultiplier);

	UFUNCTION(BlueprintCallable, Category = "CB")
	void RevertMovementSpeedMultiplier();

	UFUNCTION()
	void OnCapsuleOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBeginPatrolTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndPatrolTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnBeginAttackTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndAttackTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	int CurrentPatrolPointIndex = -1;
	float SpeedMultiplier = 1.0f;
	float InitialMaxWalkSpeed;
};
