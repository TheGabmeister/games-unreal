#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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
	bool IsDead() const { return CurrentHP <= 0.f; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MaxHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY()
	TObjectPtr<AActor> AttackTarget;

private:
	bool bIsAttacking = false;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
