#include "Enemy/RangedEnemy.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CBPlayerCharacter.h"
#include "Projectiles/ProjectileBase.h"

AEnemyRanged::AEnemyRanged()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyRanged::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MC = GetCharacterMovement())
	{
		MC->SetDefaultMovementMode();
	}
}

void AEnemyRanged::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == ERangedEnemyState::Dead) return;

	StateTimer += DeltaTime;

	switch (CurrentState)
	{
	case ERangedEnemyState::Idle:
		if (StateTimer >= ThrowInterval)
		{
			CurrentState = ERangedEnemyState::Throwing;
			StateTimer = 0.0f;
		}
		break;

	case ERangedEnemyState::Throwing:
		if (StateTimer >= ThrowWindupTime)
		{
			SpawnProjectile();
			CurrentState = ERangedEnemyState::Idle;
			StateTimer = 0.0f;
		}
		break;

	default:
		break;
	}
}

void AEnemyRanged::SpawnProjectile()
{
	if (!ProjectileClass) return;

	FVector SpawnLocation = GetActorLocation() + GetActorRotation().RotateVector(ProjectileSpawnOffset);
	FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

	if (AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, SpawnTransform))
	{
		Projectile->SetInstigator(this);

		FVector Direction;
		if (bAimAtPlayer)
		{
			if (ACBPlayerCharacter* Player = Cast<ACBPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
			{
				Direction = (Player->GetActorLocation() - SpawnLocation).GetSafeNormal();
			}
			else
			{
				Direction = GetActorForwardVector();
			}
		}
		else
		{
			Direction = GetActorForwardVector();
		}

		Projectile->InitDirection(Direction);
	}
}

void AEnemyRanged::HandleDefeat()
{
	CurrentState = ERangedEnemyState::Dead;
	Super::HandleDefeat();
}
