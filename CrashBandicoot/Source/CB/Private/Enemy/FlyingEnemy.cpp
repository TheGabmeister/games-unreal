#include "Enemy/FlyingEnemy.h"

#include "Components/SplineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyFlying::AEnemyFlying()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyFlying::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MC = GetCharacterMovement())
	{
		MC->GravityScale = 0.0f;
		MC->SetMovementMode(MOVE_Flying);
	}

	PerchLocation = GetActorLocation();

	if (bIsSwooper)
	{
		CurrentState = EFlyingEnemyState::Perched;
	}
	else
	{
		CurrentState = EFlyingEnemyState::Flying;
	}
}

void AEnemyFlying::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EFlyingEnemyState::Dead) return;

	switch (CurrentState)
	{
	case EFlyingEnemyState::Flying:
		TickFlying(DeltaTime);
		break;
	case EFlyingEnemyState::Perched:
		// Wait for trigger — attack trigger overlap starts swoop
		break;
	case EFlyingEnemyState::Swooping:
		TickSwooping(DeltaTime);
		break;
	case EFlyingEnemyState::Returning:
		TickReturning(DeltaTime);
		break;
	default:
		break;
	}
}

void AEnemyFlying::TickFlying(float DeltaTime)
{
	if (!PatrolSpline) return;

	CurrentSplineDistance += FlySpeed * DeltaTime;
	float SplineLength = PatrolSpline->GetSplineLength();
	if (SplineLength > 0.0f)
	{
		CurrentSplineDistance = FMath::Fmod(CurrentSplineDistance, SplineLength);
	}

	FVector NewLocation = PatrolSpline->GetLocationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World);
	FVector Direction = (NewLocation - GetActorLocation()).GetSafeNormal();
	SetActorLocation(NewLocation);

	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.Rotation());
	}
}

void AEnemyFlying::TickSwooping(float DeltaTime)
{
	StateAlpha += DeltaTime / SwoopDuration;

	if (StateAlpha >= 1.0f)
	{
		CurrentState = EFlyingEnemyState::Returning;
		StateAlpha = 0.0f;
		return;
	}

	float EasedAlpha = FMath::InterpEaseIn(0.0f, 1.0f, StateAlpha, 2.0f);
	FVector SwoopTarget = PerchLocation - FVector(0.0f, 0.0f, SwoopDepth);
	SetActorLocation(FMath::Lerp(PerchLocation, SwoopTarget, EasedAlpha));
}

void AEnemyFlying::TickReturning(float DeltaTime)
{
	StateAlpha += DeltaTime / ReturnDuration;

	if (StateAlpha >= 1.0f)
	{
		CurrentState = EFlyingEnemyState::Perched;
		StateAlpha = 0.0f;
		SetActorLocation(PerchLocation);
		return;
	}

	FVector SwoopTarget = PerchLocation - FVector(0.0f, 0.0f, SwoopDepth);
	SetActorLocation(FMath::Lerp(SwoopTarget, PerchLocation, StateAlpha));
}

void AEnemyFlying::HandleDefeat()
{
	CurrentState = EFlyingEnemyState::Dead;
	Super::HandleDefeat();
}
