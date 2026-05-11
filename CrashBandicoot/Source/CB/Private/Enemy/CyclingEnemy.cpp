#include "Enemy/CyclingEnemy.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Player/CBPlayerCharacter.h"

AEnemyCycling::AEnemyCycling()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyCycling::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == ECyclingEnemyState::Dead) return;

	StateTimer += DeltaTime;

	if (CurrentState == ECyclingEnemyState::Vulnerable && StateTimer >= VulnerableDuration)
	{
		SetState(ECyclingEnemyState::Attacking);
	}
	else if (CurrentState == ECyclingEnemyState::Attacking && StateTimer >= AttackingDuration)
	{
		SetState(ECyclingEnemyState::Vulnerable);
	}

	if (bPatrolsWhileAttacking && CurrentState == ECyclingEnemyState::Attacking)
	{
		FVector CurrentLocation = GetActorLocation();
		float Dist = FVector::Dist2D(CurrentLocation, CurrentPatrolTarget);
		if (Dist <= PatrolPointReachedThreshold)
		{
			CurrentPatrolTarget = GetNextPatrolLocation();
		}
		FVector Direction = (CurrentPatrolTarget - CurrentLocation).GetSafeNormal2D();
		AddMovementInput(Direction, 1.0f);
	}
}

void AEnemyCycling::SetState(ECyclingEnemyState NewState)
{
	CurrentState = NewState;
	StateTimer = 0.0f;

	if (NewState == ECyclingEnemyState::Attacking && bPatrolsWhileAttacking)
	{
		CurrentPatrolTarget = GetNextPatrolLocation();
		if (UCharacterMovementComponent* MC = GetCharacterMovement())
		{
			MC->SetDefaultMovementMode();
		}
	}
	else if (NewState == ECyclingEnemyState::Vulnerable && bPatrolsWhileAttacking)
	{
		if (UCharacterMovementComponent* MC = GetCharacterMovement())
		{
			MC->DisableMovement();
		}
	}
}

void AEnemyCycling::OnSpinHit(ACBPlayerCharacter* Player)
{
	if (IsDead()) return;

	if (bInvulnerableWhileAttacking && CurrentState == ECyclingEnemyState::Attacking)
	{
		return;
	}

	Super::OnSpinHit(Player);
}

void AEnemyCycling::OnJumpHit(ACBPlayerCharacter* Player)
{
	if (IsDead()) return;

	if (bSpikedTop)
	{
		Player->OnHit(this);
		return;
	}

	if (bInvulnerableWhileAttacking && CurrentState == ECyclingEnemyState::Attacking)
	{
		Player->LaunchCharacter(FVector(0.f, 0.f, Player->StompBounceVelocity), false, true);
		return;
	}

	Super::OnJumpHit(Player);
}

void AEnemyCycling::HandleDefeat()
{
	CurrentState = ECyclingEnemyState::Dead;
	Super::HandleDefeat();
}
