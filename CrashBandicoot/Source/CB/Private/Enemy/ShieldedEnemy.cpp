#include "Enemy/ShieldedEnemy.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CBPlayerCharacter.h"

AEnemyShielded::AEnemyShielded()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyShielded::BeginPlay()
{
	Super::BeginPlay();
	CurrentPatrolTarget = GetActorLocation();
}

void AEnemyShielded::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EPatrolEnemyState::Dead) return;

	UCharacterMovementComponent* MC = GetCharacterMovement();
	if (!MC) return;

	if (MC->GetMovementName() == TEXT("None"))
	{
		if (CurrentState != EPatrolEnemyState::Idle)
		{
			CurrentState = EPatrolEnemyState::Idle;
		}
		return;
	}

	if (CurrentState == EPatrolEnemyState::Idle)
	{
		CurrentState = EPatrolEnemyState::Patrolling;
		CurrentPatrolTarget = GetNextPatrolLocation();
	}

	TickPatrolling(DeltaTime);
}

void AEnemyShielded::TickPatrolling(float DeltaTime)
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

void AEnemyShielded::OnSpinHit(ACBPlayerCharacter* Player)
{
	if (IsDead()) return;

	if (IsSpinBlockedByShield(Player))
	{
		FVector BounceDir = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		Player->LaunchCharacter(BounceDir * SpinBouncebackForce, true, false);

		if (ShieldBlockSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ShieldBlockSound, GetActorLocation());
		}
		return;
	}

	Super::OnSpinHit(Player);
}

bool AEnemyShielded::IsSpinBlockedByShield(ACBPlayerCharacter* Player) const
{
	FVector ShieldForward = GetActorForwardVector();
	FVector ToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	float Dot = FVector::DotProduct(ShieldForward, ToPlayer);
	float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(ShieldHalfAngle));
	return Dot > CosHalfAngle;
}

void AEnemyShielded::HandleDefeat()
{
	CurrentState = EPatrolEnemyState::Dead;
	Super::HandleDefeat();
}
