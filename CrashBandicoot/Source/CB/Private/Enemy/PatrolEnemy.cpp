#include "Enemy/PatrolEnemy.h"

#include "GameFramework/CharacterMovementComponent.h"

AEnemyPatrol::AEnemyPatrol()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyPatrol::BeginPlay()
{
	Super::BeginPlay();
	CurrentPatrolTarget = GetActorLocation();
}

void AEnemyPatrol::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EPatrolEnemyState::Dead) return;

	UCharacterMovementComponent* MC = GetCharacterMovement();
	if (!MC) return;

	if (MC->GetMovementName() == TEXT("None"))
	{
		if (CurrentState != EPatrolEnemyState::Idle)
		{
			SetState(EPatrolEnemyState::Idle);
		}
		return;
	}

	if (CurrentState == EPatrolEnemyState::Idle)
	{
		SetState(EPatrolEnemyState::Patrolling);
	}

	TickPatrolling(DeltaTime);
}

void AEnemyPatrol::SetState(EPatrolEnemyState NewState)
{
	CurrentState = NewState;

	if (NewState == EPatrolEnemyState::Patrolling)
	{
		CurrentPatrolTarget = GetNextPatrolLocation();
	}
}

void AEnemyPatrol::TickPatrolling(float DeltaTime)
{
	FVector CurrentLocation = GetActorLocation();
	float DistToTarget = FVector::Dist2D(CurrentLocation, CurrentPatrolTarget);

	if (DistToTarget <= PatrolPointReachedThreshold)
	{
		CurrentPatrolTarget = GetNextPatrolLocation();
	}

	FVector Direction = (CurrentPatrolTarget - CurrentLocation).GetSafeNormal2D();
	AddMovementInput(Direction, 1.0f);
}

void AEnemyPatrol::HandleDefeat()
{
	CurrentState = EPatrolEnemyState::Dead;
	Super::HandleDefeat();
}
