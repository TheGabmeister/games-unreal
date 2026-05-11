#pragma once

#include "CoreMinimal.h"
#include "Enemy/CBEnemyCharacterBase.h"
#include "TurtleEnemy.generated.h"

UENUM(BlueprintType)
enum class ETurtleEnemyState : uint8 { Idle, Patrolling, Flipped, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyTurtle : public ACBEnemyCharacterBase
{
	GENERATED_BODY()

public:
	AEnemyTurtle();
	virtual void Tick(float DeltaTime) override;
	virtual void OnSpinHit(ACBPlayerCharacter* Player) override;
	virtual void OnJumpHit(ACBPlayerCharacter* Player) override;

protected:
	ETurtleEnemyState CurrentState = ETurtleEnemyState::Idle;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float FlippedDuration = 5.0f;

	UPROPERTY(VisibleAnywhere, Category = "CB")
	TObjectPtr<UStaticMeshComponent> PlatformComponent;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float PatrolPointReachedThreshold = 50.0f;

	void SetState(ETurtleEnemyState NewState);
	virtual void HandleDefeat() override;
	void FlipUp();
	float FlippedTimer = 0.0f;
	FVector CurrentPatrolTarget;

	virtual void BeginPlay() override;
	void TickPatrolling(float DeltaTime);
};
