#include "Enemy/CeilingEnemy.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyCeiling::AEnemyCeiling()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyCeiling::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MC = GetCharacterMovement())
	{
		MC->GravityScale = 0.0f;
		MC->SetMovementMode(MOVE_Flying);
	}

	CeilingLocation = GetActorLocation();

	// Line trace to find ground
	FHitResult Hit;
	FVector Start = CeilingLocation;
	FVector End = Start - FVector(0.0f, 0.0f, 5000.0f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
	{
		GroundLocation = Hit.ImpactPoint + FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	}
	else
	{
		GroundLocation = CeilingLocation - FVector(0.0f, 0.0f, 500.0f);
	}
}

void AEnemyCeiling::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == ECeilingEnemyState::Dead) return;

	switch (CurrentState)
	{
	case ECeilingEnemyState::Hanging:
	{
		// Check if attack trigger has been activated
		if (bTriggered)
		{
			SetState(ECeilingEnemyState::Dropping);
			bTriggered = false;
		}
		break;
	}

	case ECeilingEnemyState::Dropping:
	{
		FVector Current = GetActorLocation();
		FVector NewLocation = FMath::VInterpConstantTo(Current, GroundLocation, DeltaTime, DropSpeed);
		SetActorLocation(NewLocation);

		if (FVector::Dist(NewLocation, GroundLocation) < 5.0f)
		{
			SetActorLocation(GroundLocation);
			SetState(ECeilingEnemyState::Landed);
		}
		break;
	}

	case ECeilingEnemyState::Landed:
	{
		StateTimer += DeltaTime;
		if (StateTimer >= LandedPauseDuration)
		{
			SetState(ECeilingEnemyState::Climbing);
		}
		break;
	}

	case ECeilingEnemyState::Climbing:
	{
		FVector Current = GetActorLocation();
		FVector NewLocation = FMath::VInterpConstantTo(Current, CeilingLocation, DeltaTime, ClimbSpeed);
		SetActorLocation(NewLocation);

		if (FVector::Dist(NewLocation, CeilingLocation) < 5.0f)
		{
			SetActorLocation(CeilingLocation);
			SetState(ECeilingEnemyState::Hanging);
		}
		break;
	}

	default:
		break;
	}
}

void AEnemyCeiling::SetState(ECeilingEnemyState NewState)
{
	CurrentState = NewState;
	StateTimer = 0.0f;
}

void AEnemyCeiling::HandleDefeat()
{
	CurrentState = ECeilingEnemyState::Dead;
	Super::HandleDefeat();
}
