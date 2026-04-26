#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DiabloAIController.generated.h"

UENUM()
enum class EAIState : uint8
{
	Idle,
	Chase,
	Flee,
	Attack,
	Dead
};

UCLASS()
class DIABLO_API ADiabloAIController : public AAIController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float AggroRange = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float AttackRange = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float LeashRange = 1500.f;

private:
	EAIState State = EAIState::Idle;

	void TickIdle(float DeltaTime);
	void TickChase(float DeltaTime);
	void TickFlee(float DeltaTime);
	void TickAttack(float DeltaTime);

	APawn* FindTarget() const;
	void SetState(EAIState NewState);
	void StartFlee(float Duration, bool bConsumeLowHealthFlee);

	float FleeEndTime = 0.f;
	bool bUsedLowHealthFlee = false;
};
