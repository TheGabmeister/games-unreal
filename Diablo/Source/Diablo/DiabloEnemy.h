#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DiabloEnemy.generated.h"

UCLASS(Abstract)
class DIABLO_API ADiabloEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	ADiabloEnemy();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	bool IsDead() const { return CurrentHP <= 0.f; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MaxHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHP;
};
