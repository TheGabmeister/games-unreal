#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DiabloStats.h"
#include "DiabloEnemy.generated.h"

class UAnimMontage;

UCLASS(Abstract)
class DIABLO_API ADiabloEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	ADiabloEnemy();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	void StartAttack(AActor* Target);
	bool IsAttacking() const { return bIsAttacking; }
	bool IsDead() const { return !Stats.IsAlive(); }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	FDiabloStats Stats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	int64 XPReward = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	int32 MonsterLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY()
	TObjectPtr<AActor> AttackTarget;

private:
	bool bIsAttacking = false;
	FTimerHandle DestroyTimerHandle;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void OnDestroyTimer();
};
