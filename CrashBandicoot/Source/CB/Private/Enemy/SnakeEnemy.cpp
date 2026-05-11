#include "Enemy/SnakeEnemy.h"

#include "Components/CapsuleComponent.h"
#include "Player/CBPlayerCharacter.h"

AEnemySnake::AEnemySnake()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemySnake::BeginPlay()
{
	Super::BeginPlay();
	HoleLocation = GetActorLocation();

	SetState(ESnakeEnemyState::Hidden);
}

void AEnemySnake::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == ESnakeEnemyState::Dead) return;

	StateTimer += DeltaTime;

	switch (CurrentState)
	{
	case ESnakeEnemyState::Hidden:
		if (StateTimer >= HiddenDuration)
		{
			SetState(ESnakeEnemyState::Emerging);
		}
		break;

	case ESnakeEnemyState::Emerging:
		if (StateTimer >= EmergeDuration)
		{
			SetState(ESnakeEnemyState::Lunging);
		}
		break;

	case ESnakeEnemyState::Lunging:
	{
		float Alpha = FMath::Clamp(StateTimer / LungeDuration, 0.0f, 1.0f);
		FVector TargetLocation = HoleLocation + LungeDirection * LungeDistance;
		SetActorLocation(FMath::Lerp(HoleLocation, TargetLocation, Alpha));

		if (StateTimer >= LungeDuration)
		{
			SetState(ESnakeEnemyState::Hidden);
		}
		break;
	}
	default:
		break;
	}
}

void AEnemySnake::SetState(ESnakeEnemyState NewState)
{
	CurrentState = NewState;
	StateTimer = 0.0f;

	switch (NewState)
	{
	case ESnakeEnemyState::Hidden:
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetActorHiddenInGame(true);
		SetActorLocation(HoleLocation);
		break;

	case ESnakeEnemyState::Emerging:
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SetActorHiddenInGame(false);
		break;

	case ESnakeEnemyState::Lunging:
		break;

	case ESnakeEnemyState::Dead:
		break;
	}
}

void AEnemySnake::OnSpinHit(ACBPlayerCharacter* Player)
{
	if (IsDead() || CurrentState == ESnakeEnemyState::Hidden) return;
	Super::OnSpinHit(Player);
}

void AEnemySnake::OnJumpHit(ACBPlayerCharacter* Player)
{
	if (IsDead() || CurrentState == ESnakeEnemyState::Hidden) return;
	Super::OnJumpHit(Player);
}

void AEnemySnake::HandleDefeat()
{
	CurrentState = ESnakeEnemyState::Dead;
	Super::HandleDefeat();
}
