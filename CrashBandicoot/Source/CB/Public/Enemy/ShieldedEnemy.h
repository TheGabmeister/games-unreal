#pragma once

#include "CoreMinimal.h"
#include "Enemy/CBEnemyCharacterBase.h"
#include "Enemy/PatrolEnemy.h"
#include "ShieldedEnemy.generated.h"

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyShielded : public ACBEnemyCharacterBase
{
	GENERATED_BODY()

public:
	AEnemyShielded();
	virtual void Tick(float DeltaTime) override;
	virtual void OnSpinHit(ACBPlayerCharacter* Player) override;

protected:
	EPatrolEnemyState CurrentState = EPatrolEnemyState::Idle;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float ShieldHalfAngle = 90.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float SpinBouncebackForce = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<USoundBase> ShieldBlockSound;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float PatrolPointReachedThreshold = 50.0f;

	bool IsSpinBlockedByShield(ACBPlayerCharacter* Player) const;

	FVector CurrentPatrolTarget;

	virtual void HandleDefeat() override;
	virtual void BeginPlay() override;
	void TickPatrolling(float DeltaTime);
};
