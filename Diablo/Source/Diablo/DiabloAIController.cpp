#include "DiabloAIController.h"
#include "DiabloEnemy.h"
#include "DiabloHero.h"
#include "Diablo.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

void ADiabloAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	SetState(EAIState::Idle);
}

void ADiabloAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ADiabloEnemy* Enemy = Cast<ADiabloEnemy>(GetPawn());
	if (!Enemy || Enemy->IsDead())
	{
		SetState(EAIState::Dead);
		return;
	}

	switch (State)
	{
	case EAIState::Idle:   TickIdle(DeltaTime);   break;
	case EAIState::Chase:  TickChase(DeltaTime);   break;
	case EAIState::Attack: TickAttack(DeltaTime);  break;
	case EAIState::Dead:   break;
	}
}

void ADiabloAIController::TickIdle(float DeltaTime)
{
	APawn* Target = FindTarget();
	if (!Target)
	{
		return;
	}

	const float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
	if (Distance <= AggroRange)
	{
		UE_LOG(LogDiablo, Display, TEXT("%s: aggro on %s (%.0f <= %.0f)"),
			*GetPawn()->GetName(), *Target->GetName(), Distance, AggroRange);
		SetState(EAIState::Chase);
	}
}

void ADiabloAIController::TickChase(float DeltaTime)
{
	APawn* Target = FindTarget();
	if (!Target)
	{
		SetState(EAIState::Idle);
		return;
	}

	const float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());

	if (Distance > LeashRange)
	{
		UE_LOG(LogDiablo, Display, TEXT("%s: target out of leash range, returning to idle"), *GetPawn()->GetName());
		StopMovement();
		SetState(EAIState::Idle);
		return;
	}

	if (Distance <= AttackRange)
	{
		StopMovement();
		SetState(EAIState::Attack);
		return;
	}

	MoveToActor(Target, AttackRange * 0.8f);
}

void ADiabloAIController::TickAttack(float DeltaTime)
{
	APawn* Target = FindTarget();
	if (!Target)
	{
		SetState(EAIState::Idle);
		return;
	}

	ADiabloEnemy* Enemy = Cast<ADiabloEnemy>(GetPawn());
	if (!Enemy)
	{
		return;
	}

	const float Distance = FVector::Dist(Enemy->GetActorLocation(), Target->GetActorLocation());

	if (Distance > AttackRange)
	{
		SetState(EAIState::Chase);
		return;
	}

	const FVector Direction = (Target->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal2D();
	if (!Direction.IsNearlyZero())
	{
		Enemy->SetActorRotation(Direction.Rotation());
	}

	if (!Enemy->IsAttacking())
	{
		Enemy->StartAttack(Target);
	}
}

APawn* ADiabloAIController::FindTarget() const
{
	return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void ADiabloAIController::SetState(EAIState NewState)
{
	if (State == NewState)
	{
		return;
	}

	State = NewState;
}
