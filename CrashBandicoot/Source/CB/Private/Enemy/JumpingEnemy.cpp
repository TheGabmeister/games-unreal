#include "Enemy/JumpingEnemy.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Player/CBPlayerCharacter.h"

AEnemyJumping::AEnemyJumping()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyJumping::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MC = GetCharacterMovement())
	{
		MC->SetDefaultMovementMode();
	}
}

void AEnemyJumping::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EJumpingEnemyState::Dead) return;

	if (CurrentState == EJumpingEnemyState::Grounded)
	{
		StateTimer += DeltaTime;
		if (StateTimer >= GroundedDuration)
		{
			StateTimer = 0.0f;
			CurrentState = EJumpingEnemyState::Jumping;

			FVector JumpVelocity = GetActorForwardVector() * JumpImpulseForward;
			JumpVelocity.Z = JumpImpulseZ;
			LaunchCharacter(JumpVelocity, true, true);
		}
	}
}

void AEnemyJumping::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (CurrentState == EJumpingEnemyState::Jumping)
	{
		CurrentState = EJumpingEnemyState::Grounded;
		StateTimer = 0.0f;
	}
}

void AEnemyJumping::OnJumpHit(ACBPlayerCharacter* Player)
{
	if (IsDead()) return;

	float BounceVelocity = bExtraHighBounce ? ExtraHighBounceVelocity : Player->StompBounceVelocity;
	Player->LaunchCharacter(FVector(0.f, 0.f, BounceVelocity), false, true);
	KillCharacter();
	HandleDefeat();
}

void AEnemyJumping::HandleDefeat()
{
	CurrentState = EJumpingEnemyState::Dead;
	Super::HandleDefeat();
}
