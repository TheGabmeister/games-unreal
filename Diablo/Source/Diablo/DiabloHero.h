#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DiabloHero.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;

UCLASS(Abstract)
class DIABLO_API ADiabloHero : public ACharacter
{
	GENERATED_BODY()

public:
	ADiabloHero();

	void StartAttack();
	bool IsAttacking() const { return bIsAttacking; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

private:
	bool bIsAttacking = false;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
