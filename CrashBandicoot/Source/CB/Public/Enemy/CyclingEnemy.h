#pragma once

#include "CoreMinimal.h"
#include "Enemy/CBEnemyCharacterBase.h"
#include "CyclingEnemy.generated.h"

UENUM(BlueprintType)
enum class ECyclingEnemyState : uint8 { Vulnerable, Attacking, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyCycling : public ACBEnemyCharacterBase
{
	GENERATED_BODY()

public:
	AEnemyCycling();
	virtual void Tick(float DeltaTime) override;
	virtual void OnSpinHit(ACBPlayerCharacter* Player) override;
	virtual void OnJumpHit(ACBPlayerCharacter* Player) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "CB")
	ECyclingEnemyState CurrentState = ECyclingEnemyState::Vulnerable;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float VulnerableDuration = 2.5f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float AttackingDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	bool bSpikedTop = false;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	bool bPatrolsWhileAttacking = false;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	bool bOnlyDangerousWhileAttacking = true;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	bool bInvulnerableWhileAttacking = false;

	void SetState(ECyclingEnemyState NewState);
	virtual void HandleDefeat() override;
	float StateTimer = 0.0f;

	FVector CurrentPatrolTarget;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float PatrolPointReachedThreshold = 50.0f;
};
