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
class ASpellProjectile;
class USpellDefinition;

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
	void RestoreMana(float Amount);
	bool CastSpellFromScroll(USpellDefinition* SpellDef);
	void AwardXP(int64 Amount);
	bool SpendStatPoint(FName StatName);
	bool CastSpell(const FVector& TargetLocation);
	void SetActiveSpell(USpellDefinition* Spell);
	USpellDefinition* GetActiveSpell() const { return ActiveSpell; }

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spell")
	TArray<TObjectPtr<USpellDefinition>> KnownSpells;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell")
	TObjectPtr<USpellDefinition> ActiveSpell;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float EquipMinDamage = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float EquipMaxDamage = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float ArmorFromEquipment = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float ToHitFromEquipment = 0.f;

	void SaveToGameInstance();
	void LoadFromGameInstance();

	int64 GetXPForLevel(int32 Level) const;
	int64 GetXPForNextLevel() const;
	float GetXPPercent() const;
	void RecomputeDerivedStats();

private:
	bool bIsAttacking = false;
	float SpellCooldownRemaining = 0.f;

	virtual void BeginPlay() override;
	void Die();
	void LevelUp();

	static const TArray<int64>& GetXPTable();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
