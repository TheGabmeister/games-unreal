#pragma once

#include "CoreMinimal.h"
#include "Enemy/CBEnemyCharacterBase.h"
#include "PatrolEnemy.generated.h"

UENUM(BlueprintType)
enum class EPatrolEnemyState : uint8 { Idle, Patrolling, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyPatrol : public ACBEnemyCharacterBase
{
	GENERATED_BODY()

public:
	AEnemyPatrol();
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "CB")
	EPatrolEnemyState CurrentState = EPatrolEnemyState::Idle;

	void SetState(EPatrolEnemyState NewState);
	void TickPatrolling(float DeltaTime);

	FVector CurrentPatrolTarget;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float PatrolPointReachedThreshold = 50.0f;

	virtual void BeginPlay() override;
	virtual void HandleDefeat() override;
};
