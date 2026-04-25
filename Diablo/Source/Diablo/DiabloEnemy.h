#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DiabloStats.h"
#include "DiabloEnemy.generated.h"

class UAnimMontage;
class UItemDefinition;

USTRUCT(BlueprintType)
struct FDropTableEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UItemDefinition> ItemDef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropChance = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 Weight = 1;
};

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MonsterAC = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	TArray<FDropTableEntry> DropTable;

	UPROPERTY()
	TObjectPtr<AActor> AttackTarget;

private:
	void SpawnDrops();
	bool bIsAttacking = false;
	FTimerHandle DestroyTimerHandle;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void OnDestroyTimer();
};
