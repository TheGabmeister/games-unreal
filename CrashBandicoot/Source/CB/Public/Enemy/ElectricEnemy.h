#pragma once

#include "CoreMinimal.h"
#include "Enemy/CBEnemyCharacterBase.h"
#include "ElectricEnemy.generated.h"

UENUM(BlueprintType)
enum class EElectricEnemyState : uint8 { Idle, Patrolling, Pursuing, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyElectric : public ACBEnemyCharacterBase
{
	GENERATED_BODY()

public:
	AEnemyElectric();
	virtual void Tick(float DeltaTime) override;
	virtual void OnSpinHit(ACBPlayerCharacter* Player) override;
	virtual void OnJumpHit(ACBPlayerCharacter* Player) override;

protected:
	EElectricEnemyState CurrentState = EElectricEnemyState::Idle;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float PursuitSpeedMultiplier = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float ElectricBarrierHeight = 40.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float PatrolPointReachedThreshold = 50.0f;

	TWeakObjectPtr<ACBPlayerCharacter> PursuitTarget;

	FVector CurrentPatrolTarget;

	virtual void HandleDefeat() override;
	virtual void BeginPlay() override;
	void TickPatrolling(float DeltaTime);
	void TickPursuing(float DeltaTime);
};
