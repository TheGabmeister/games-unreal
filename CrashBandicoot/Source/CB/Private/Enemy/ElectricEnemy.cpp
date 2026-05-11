#include "Enemy/ElectricEnemy.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CBPlayerCharacter.h"

AEnemyElectric::AEnemyElectric()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyElectric::BeginPlay()
{
	Super::BeginPlay();
	CurrentPatrolTarget = GetActorLocation();
}

void AEnemyElectric::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EElectricEnemyState::Dead) return;

	UCharacterMovementComponent* MC = GetCharacterMovement();
	if (!MC) return;

	if (MC->GetMovementName() == TEXT("None"))
	{
		if (CurrentState != EElectricEnemyState::Idle)
		{
			CurrentState = EElectricEnemyState::Idle;
		}
		return;
	}

	switch (CurrentState)
	{
	case EElectricEnemyState::Idle:
		CurrentState = EElectricEnemyState::Patrolling;
		CurrentPatrolTarget = GetNextPatrolLocation();
		break;

	case EElectricEnemyState::Patrolling:
		TickPatrolling(DeltaTime);
		break;

	case EElectricEnemyState::Pursuing:
		TickPursuing(DeltaTime);
		break;

	default:
		break;
	}
}

void AEnemyElectric::TickPatrolling(float DeltaTime)
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

void AEnemyElectric::TickPursuing(float DeltaTime)
{
	if (!PursuitTarget.IsValid()) return;

	FVector Direction = (PursuitTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	AddMovementInput(Direction, PursuitSpeedMultiplier);
}

void AEnemyElectric::OnSpinHit(ACBPlayerCharacter* Player)
{
	if (IsDead()) return;

	float BarrierZ = GetActorLocation().Z + ElectricBarrierHeight;
	if (Player->GetActorLocation().Z < BarrierZ)
	{
		Super::OnSpinHit(Player);
	}
	else
	{
		Player->OnHit(this);
	}
}

void AEnemyElectric::OnJumpHit(ACBPlayerCharacter* Player)
{
	if (IsDead()) return;
	Player->OnHit(this);
}

void AEnemyElectric::HandleDefeat()
{
	CurrentState = EElectricEnemyState::Dead;
	Super::HandleDefeat();
}
