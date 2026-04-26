#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DiabloStats.h"
#include "DiabloEnemy.generated.h"

class UAnimMontage;
class UItemDefinition;
class ASpellProjectile;

UENUM(BlueprintType)
enum class EDiabloEnemyArchetype : uint8
{
	MeleeGrunt,
	FastMelee,
	RangedArcher,
	Spellcaster,
	Summoner,
	FallenCoward
};

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
	bool StartSpecialAttack(AActor* Target);
	bool TrySummonMinion(AActor* Target);
	void ConfigureArchetype(EDiabloEnemyArchetype NewArchetype);
	void ApplyArchetypeDefaults();
	bool IsAttacking() const { return bIsAttacking; }
	bool IsDead() const { return !Stats.IsAlive(); }
	float GetHealthPercent() const;
	bool IsRangedArchetype() const;
	bool IsCasterArchetype() const;
	bool IsSummonerArchetype() const;
	bool ShouldFlee() const;
	bool CanUsePrimaryAttack() const;
	void MarkPrimaryAttackUsed();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	EDiabloEnemyArchetype Archetype = EDiabloEnemyArchetype::MeleeGrunt;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AggroRange = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AttackRange = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PreferredRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float LeashRange = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AttackCooldown = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Flee")
	float FleeHealthPercent = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Flee")
	float FleeDuration = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ranged")
	TSubclassOf<ASpellProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ranged")
	float ProjectileDamage = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ranged")
	float ProjectileSpeed = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Summon")
	TSubclassOf<ADiabloEnemy> SummonClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Summon")
	int32 MaxSummons = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Summon")
	float SummonCooldown = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Summon")
	float SummonRadius = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Summon")
	bool bIsSummonedMinion = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	TArray<FDropTableEntry> DropTable;

	UPROPERTY()
	TObjectPtr<AActor> AttackTarget;

protected:
	virtual void BeginPlay() override;

private:
	void SpawnDrops();
	bool FireProjectileAt(AActor* Target);
	int32 GetAliveSummonCount();

	bool bIsAttacking = false;
	float LastPrimaryAttackTime = -1000.f;
	float LastSummonTime = -1000.f;
	FTimerHandle DestroyTimerHandle;
	TArray<TWeakObjectPtr<ADiabloEnemy>> SpawnedSummons;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void OnDestroyTimer();
};
