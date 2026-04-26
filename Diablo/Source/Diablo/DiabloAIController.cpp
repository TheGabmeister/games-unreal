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
	case EAIState::Flee:   TickFlee(DeltaTime);    break;
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

	ADiabloEnemy* Enemy = Cast<ADiabloEnemy>(GetPawn());
	if (!Enemy)
	{
		return;
	}

	const float Distance = FVector::Dist(Enemy->GetActorLocation(), Target->GetActorLocation());
	if (Distance <= Enemy->AggroRange)
	{
		UE_LOG(LogDiablo, Display, TEXT("%s: aggro on %s (%.0f <= %.0f)"),
			*Enemy->GetName(), *Target->GetName(), Distance, Enemy->AggroRange);
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

	ADiabloEnemy* Enemy = Cast<ADiabloEnemy>(GetPawn());
	if (!Enemy)
	{
		return;
	}

	const float Distance = FVector::Dist(Enemy->GetActorLocation(), Target->GetActorLocation());

	if (Distance > Enemy->LeashRange)
	{
		UE_LOG(LogDiablo, Display, TEXT("%s: target out of leash range, returning to idle"), *Enemy->GetName());
		StopMovement();
		SetState(EAIState::Idle);
		return;
	}

	if (Enemy->ShouldFlee() && !bUsedLowHealthFlee)
	{
		StartFlee(Enemy->FleeDuration, true);
		return;
	}

	if (Distance <= Enemy->AttackRange)
	{
		StopMovement();
		SetState(EAIState::Attack);
		return;
	}

	if (GetMoveStatus() != EPathFollowingStatus::Moving)
	{
		const float AcceptanceRadius = Enemy->PreferredRange > 0.f ?
			Enemy->PreferredRange : Enemy->AttackRange * 0.5f;
		MoveToActor(Target, AcceptanceRadius);
	}
}

void ADiabloAIController::TickFlee(float DeltaTime)
{
	APawn* Target = FindTarget();
	ADiabloEnemy* Enemy = Cast<ADiabloEnemy>(GetPawn());
	if (!Target || !Enemy)
	{
		SetState(EAIState::Idle);
		return;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (Now >= FleeEndTime)
	{
		SetState(EAIState::Chase);
		return;
	}

	if (GetMoveStatus() == EPathFollowingStatus::Moving)
	{
		return;
	}

	FVector Away = (Enemy->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal2D();
	if (Away.IsNearlyZero())
	{
		Away = -Enemy->GetActorForwardVector();
	}

	const FVector FleeLocation = Enemy->GetActorLocation() + Away * FMath::Max(Enemy->PreferredRange, 650.f);
	MoveToLocation(FleeLocation, 100.f);
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

	if (Distance > Enemy->AttackRange)
	{
		SetState(EAIState::Chase);
		return;
	}

	if (Enemy->ShouldFlee() && !bUsedLowHealthFlee)
	{
		StartFlee(Enemy->FleeDuration, true);
		return;
	}

	if (Enemy->PreferredRange > 0.f && Distance < Enemy->PreferredRange * 0.65f)
	{
		StartFlee(1.f, false);
		return;
	}

	const FVector Direction = (Target->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal2D();
	if (!Direction.IsNearlyZero())
	{
		Enemy->SetActorRotation(Direction.Rotation());
	}

	if (!Enemy->IsAttacking() && Enemy->CanUsePrimaryAttack())
	{
		Enemy->StartSpecialAttack(Target);
	}
}

APawn* ADiabloAIController::FindTarget() const
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (ADiabloHero* Hero = Cast<ADiabloHero>(PlayerPawn))
	{
		if (Hero->IsDead())
		{
			return nullptr;
		}
	}
	return PlayerPawn;
}

void ADiabloAIController::SetState(EAIState NewState)
{
	if (State == NewState)
	{
		return;
	}

	State = NewState;
}

void ADiabloAIController::StartFlee(float Duration, bool bConsumeLowHealthFlee)
{
	StopMovement();
	FleeEndTime = (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f) + Duration;
	if (bConsumeLowHealthFlee)
	{
		bUsedLowHealthFlee = true;
	}
	SetState(EAIState::Flee);
}
