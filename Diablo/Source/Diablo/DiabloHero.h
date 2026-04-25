#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DiabloStats.h"
#include "DiabloHero.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class USoundWave;
class ADiabloEnemy;
class UInventoryComponent;

DECLARE_MULTICAST_DELEGATE(FOnStatsChanged);

UCLASS(Abstract)
class DIABLO_API ADiabloHero : public ACharacter
{
	GENERATED_BODY()

public:
	ADiabloHero();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	void StartAttack();
	bool IsAttacking() const { return bIsAttacking; }
	bool IsDead() const { return !Stats.IsAlive(); }
	void Heal(float Amount);
	void AwardXP(int64 Amount);
	bool SpendStatPoint(FName StatName);

	FOnStatsChanged OnStatsChanged;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	FDiabloStats Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 CharLevel = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	int64 CurrentXP = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 UnspentStatPoints = 0;

	UPROPERTY()
	TObjectPtr<ADiabloEnemy> AttackTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundWave> LevelUpSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryComponent> Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float EquipMinDamage = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float EquipMaxDamage = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float ArmorFromEquipment = 0.f;

	int64 GetXPForLevel(int32 Level) const;
	int64 GetXPForNextLevel() const;
	float GetXPPercent() const;
	void RecomputeDerivedStats();

private:
	bool bIsAttacking = false;

	void Die();
	void LevelUp();

	static const TArray<int64>& GetXPTable();

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
