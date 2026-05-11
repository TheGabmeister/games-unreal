#include "Enemy/TurtleEnemy.h"

#include "CBCollisionChannels.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/CBPlayerCharacter.h"

AEnemyTurtle::AEnemyTurtle()
{
	PrimaryActorTick.bCanEverTick = true;

	PlatformComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform"));
	PlatformComponent->SetupAttachment(GetCapsuleComponent());
	PlatformComponent->SetCollisionProfileName(TEXT("BlockAll"));
	PlatformComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlatformComponent->SetVisibility(false);
	PlatformComponent->CanCharacterStepUpOn = ECB_Yes;
}

void AEnemyTurtle::BeginPlay()
{
	Super::BeginPlay();
	CurrentPatrolTarget = GetActorLocation();
}

void AEnemyTurtle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == ETurtleEnemyState::Dead) return;

	UCharacterMovementComponent* MC = GetCharacterMovement();
	if (!MC) return;

	switch (CurrentState)
	{
	case ETurtleEnemyState::Idle:
		if (MC->GetMovementName() != TEXT("None"))
		{
			SetState(ETurtleEnemyState::Patrolling);
		}
		break;

	case ETurtleEnemyState::Patrolling:
		if (MC->GetMovementName() == TEXT("None"))
		{
			CurrentState = ETurtleEnemyState::Idle;
		}
		else
		{
			TickPatrolling(DeltaTime);
		}
		break;

	case ETurtleEnemyState::Flipped:
		FlippedTimer += DeltaTime;
		// Continue patrolling while flipped
		TickPatrolling(DeltaTime);
		if (FlippedTimer >= FlippedDuration)
		{
			FlipUp();
		}
		break;

	default:
		break;
	}
}

void AEnemyTurtle::TickPatrolling(float DeltaTime)
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

void AEnemyTurtle::SetState(ETurtleEnemyState NewState)
{
	CurrentState = NewState;

	switch (NewState)
	{
	case ETurtleEnemyState::Patrolling:
		CurrentPatrolTarget = GetNextPatrolLocation();
		PlatformComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionResponseToChannel(CBCollision::Player, ECR_Overlap);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		break;

	case ETurtleEnemyState::Flipped:
		FlippedTimer = 0.0f;
		PlatformComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetCapsuleComponent()->SetCollisionResponseToChannel(CBCollision::Player, ECR_Ignore);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		GetMesh()->SetRelativeRotation(FRotator(180.0f, -90.0f, 0.0f));
		break;

	default:
		break;
	}
}

void AEnemyTurtle::FlipUp()
{
	SetState(ETurtleEnemyState::Patrolling);
}

void AEnemyTurtle::OnSpinHit(ACBPlayerCharacter* Player)
{
	if (IsDead()) return;
	Super::OnSpinHit(Player);
}

void AEnemyTurtle::OnJumpHit(ACBPlayerCharacter* Player)
{
	if (IsDead()) return;

	Player->LaunchCharacter(FVector(0.f, 0.f, Player->StompBounceVelocity), false, true);

	if (CurrentState == ETurtleEnemyState::Flipped)
	{
		FlippedTimer = 0.0f;
		return;
	}

	SetState(ETurtleEnemyState::Flipped);
}

void AEnemyTurtle::HandleDefeat()
{
	CurrentState = ETurtleEnemyState::Dead;
	Super::HandleDefeat();
}
