#pragma once

#include "CoreMinimal.h"
#include "Enemy/CBEnemyCharacterBase.h"
#include "SnakeEnemy.generated.h"

UENUM(BlueprintType)
enum class ESnakeEnemyState : uint8 { Hidden, Emerging, Lunging, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemySnake : public ACBEnemyCharacterBase
{
	GENERATED_BODY()

public:
	AEnemySnake();
	virtual void Tick(float DeltaTime) override;
	virtual void OnSpinHit(ACBPlayerCharacter* Player) override;
	virtual void OnJumpHit(ACBPlayerCharacter* Player) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "CB")
	ESnakeEnemyState CurrentState = ESnakeEnemyState::Hidden;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float HiddenDuration = 2.5f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float EmergeDuration = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float LungeDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float LungeDistance = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	FVector LungeDirection = FVector::ForwardVector;

	FVector HoleLocation;
	float StateTimer = 0.0f;

	void SetState(ESnakeEnemyState NewState);
	virtual void HandleDefeat() override;
	virtual void BeginPlay() override;
};
