#include "Enemy/GreenBlobEnemy.h"

#include "GameFramework/CharacterMovementComponent.h"

AGreenBlobEnemy::AGreenBlobEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	HitPoints = 1;
	bSpinLaunchesAsProjectile = false;
}

void AGreenBlobEnemy::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(BlobLifetime);

	if (UCharacterMovementComponent* MC = GetCharacterMovement())
	{
		MC->SetDefaultMovementMode();
	}
}

void AGreenBlobEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	BounceTimer += DeltaTime;
	if (BounceTimer >= BounceInterval)
	{
		BounceTimer = 0.0f;
		LaunchCharacter(FVector(0.f, 0.f, BounceImpulseZ), false, true);
	}
}
