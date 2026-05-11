
#include "Enemy/CBEnemyCharacterBase.h"

#include "CBCollisionChannels.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Enemy/LaunchedEnemyProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CBPlayerCharacter.h"

ACBEnemyCharacterBase::ACBEnemyCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(CBCollision::Player, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->bOrientRotationToMovement = true;
		CMC->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	}
}

void ACBEnemyCharacterBase::BeginPlay()
{
	CurrentHitPoints = HitPoints;

	if (UCharacterMovementComponent* MC = GetCharacterMovement())
	{
		MC->DisableMovement();
		InitialMaxWalkSpeed = MC->MaxWalkSpeed;
	}

	Super::BeginPlay();

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACBEnemyCharacterBase::OnCapsuleOverlap);

	// Check for already-overlapping actors in trigger volumes
	TArray<AActor*> OverlappingActors;

	if (IsValid(AttackTriggerVolume))
	{
		AttackTriggerVolume->GetOverlappingActors(OverlappingActors, ACBPlayerCharacter::StaticClass());
		if (OverlappingActors.Num() > 0)
		{
			SetMovementSpeedMultiplier(AttackSpeedMultiplier);
		}
	}

	if (IsValid(PatrolTriggerVolume))
	{
		PatrolTriggerVolume->GetOverlappingActors(OverlappingActors, ACBPlayerCharacter::StaticClass());
		if (OverlappingActors.Num() > 0)
		{
			if (UCharacterMovementComponent* MC = GetCharacterMovement())
			{
				MC->SetDefaultMovementMode();
			}
		}
	}
}

void ACBEnemyCharacterBase::HitCharacter()
{
	if (IsDead()) return;

	CurrentHitPoints = FMath::Max(CurrentHitPoints - 1, 0);

	if (IsDead())
	{
		HandleDefeat();
	}
}

void ACBEnemyCharacterBase::HitCharacterWithLaunchForce(const FVector& Force)
{
	if (IsDead()) return;
	LaunchCharacter(Force, true, true);
	HitCharacter();
}

void ACBEnemyCharacterBase::KillCharacter()
{
	if (!IsDead())
	{
		CurrentHitPoints = 0;
	}
}

void ACBEnemyCharacterBase::OnSpinHit(ACBPlayerCharacter* Player)
{
	if (IsDead()) return;

	FVector LaunchDir = (GetActorLocation() - Player->GetActorLocation()).GetSafeNormal2D();
	if (LaunchDir.IsNearlyZero())
	{
		LaunchDir = Player->GetActorForwardVector();
	}

	KillCharacter();

	if (bSpinLaunchesAsProjectile)
	{
		SpawnLaunchedProjectile(LaunchDir);
	}

	HandleDefeat();
}

void ACBEnemyCharacterBase::OnJumpHit(ACBPlayerCharacter* Player)
{
	if (IsDead()) return;

	Player->LaunchCharacter(FVector(0.f, 0.f, Player->StompBounceVelocity), false, true);
	KillCharacter();
	HandleDefeat();
}

void ACBEnemyCharacterBase::OnExplosionHit(FVector Origin, float Radius)
{
	if (IsDead()) return;

	KillCharacter();
	HandleDefeat();
}

void ACBEnemyCharacterBase::HandleDefeat()
{
	if (DefeatSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DefeatSound, GetActorLocation());
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UCharacterMovementComponent* MC = GetCharacterMovement())
	{
		MC->DisableMovement();
	}

	SetLifeSpan(0.1f);
}

void ACBEnemyCharacterBase::SpawnLaunchedProjectile(FVector LaunchDirection)
{
	FTransform SpawnTransform(FRotator::ZeroRotator, GetActorLocation());
	if (ALaunchedEnemyProjectile* Projectile = GetWorld()->SpawnActor<ALaunchedEnemyProjectile>(ALaunchedEnemyProjectile::StaticClass(), SpawnTransform))
	{
		Projectile->InitProjectile(LaunchDirection, SpinLaunchForce);
	}
}

void ACBEnemyCharacterBase::OnCapsuleOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsDead()) return;

	if (ACBPlayerCharacter* Player = Cast<ACBPlayerCharacter>(Other))
	{
		if (Player->IsDead()) return;

		if (Player->IsInvincible())
		{
			OnSpinHit(Player);
			return;
		}

		Player->OnHit(this);
	}
}

FVector ACBEnemyCharacterBase::GetNextPatrolLocation()
{
	if (!IsValid(PatrolSpline))
	{
		return FVector::ZeroVector;
	}

	IncrementPatrolPoint();
	return PatrolSpline->GetLocationAtSplinePoint(CurrentPatrolPointIndex, ESplineCoordinateSpace::World);
}

void ACBEnemyCharacterBase::IncrementPatrolPoint()
{
	if (!IsValid(PatrolSpline)) return;

	if (CurrentPatrolPointIndex + 1 == PatrolSpline->GetNumberOfSplinePoints())
	{
		CurrentPatrolPointIndex = 0;
	}
	else
	{
		CurrentPatrolPointIndex++;
	}
}

void ACBEnemyCharacterBase::SetMovementSpeedMultiplier(float NewMultiplier)
{
	SpeedMultiplier = NewMultiplier;
	if (UCharacterMovementComponent* MC = GetCharacterMovement())
	{
		MC->MaxWalkSpeed = InitialMaxWalkSpeed * SpeedMultiplier;
	}
}

void ACBEnemyCharacterBase::RevertMovementSpeedMultiplier()
{
	SetMovementSpeedMultiplier(1.0f);
}

void ACBEnemyCharacterBase::Init(FEnemyInitializationArgs InitArgs)
{
	if (auto* PrimitiveComponent = GetComponentByClass<UPrimitiveComponent>())
	{
		PrimitiveComponent->BodyInstance.bLockXTranslation = InitArgs.LockXTransform;
		PrimitiveComponent->BodyInstance.bLockYTranslation = InitArgs.LockYTransform;
		PrimitiveComponent->BodyInstance.bLockZTranslation = InitArgs.LockZTransform;
		PrimitiveComponent->BodyInstance.bLockTranslation = InitArgs.LockXTransform || InitArgs.LockYTransform || InitArgs.LockZTransform;
	}

	PatrolSpline = InitArgs.PatrolSpline;
	PatrolTriggerVolume = InitArgs.PatrolTriggerBox;
	AttackTriggerVolume = InitArgs.AttackTriggerBox;

	if (IsValid(PatrolTriggerVolume))
	{
		PatrolTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ACBEnemyCharacterBase::OnBeginPatrolTriggerOverlap);
		PatrolTriggerVolume->OnComponentEndOverlap.AddDynamic(this, &ACBEnemyCharacterBase::OnEndPatrolTriggerOverlap);
	}

	if (IsValid(AttackTriggerVolume))
	{
		AttackTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ACBEnemyCharacterBase::OnBeginAttackTriggerOverlap);
		AttackTriggerVolume->OnComponentEndOverlap.AddDynamic(this, &ACBEnemyCharacterBase::OnEndAttackTriggerOverlap);
	}
}

float ACBEnemyCharacterBase::GetMovementSpeedMultiplier()
{
	return SpeedMultiplier;
}

void ACBEnemyCharacterBase::OnBeginPatrolTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<ACBPlayerCharacter>(Other))
	{
		if (UCharacterMovementComponent* MC = GetCharacterMovement())
		{
			MC->SetDefaultMovementMode();
		}
	}
}

void ACBEnemyCharacterBase::OnEndPatrolTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<ACBPlayerCharacter>(Other))
	{
		if (UCharacterMovementComponent* MC = GetCharacterMovement())
		{
			MC->DisableMovement();
		}
	}
}

void ACBEnemyCharacterBase::OnBeginAttackTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<ACBPlayerCharacter>(Other))
	{
		SetMovementSpeedMultiplier(AttackSpeedMultiplier);
	}
}

void ACBEnemyCharacterBase::OnEndAttackTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<ACBPlayerCharacter>(Other))
	{
		RevertMovementSpeedMultiplier();
	}
}
