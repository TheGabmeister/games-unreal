#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DiabloStats.h"
#include "DiabloHero.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class ADiabloEnemy;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	FDiabloStats Stats;

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

private:
	bool bIsAttacking = false;

	void Die();

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
