#include "ChainLightning.h"
#include "DiabloEnemy.h"
#include "Diablo.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "UObject/ConstructorHelpers.h"

AChainLightning::AChainLightning()
{
	ManaCost = 15.f;
	Cooldown = 1.f;
	Damage = 25.f;
	Speed = 1800.f;

	CollisionComponent->SetSphereRadius(15.f);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.5f));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}
}

void AChainLightning::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ASpellProjectile::OnOverlap);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AChainLightning::OnChainOverlap);
}

void AChainLightning::OnChainOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetInstigator())
	{
		return;
	}

	if (PreviousHitActor.IsValid() && OtherActor == PreviousHitActor.Get())
	{
		return;
	}

	if (!bDamageEnemies)
	{
		return;
	}

	ADiabloEnemy* HitEnemy = Cast<ADiabloEnemy>(OtherActor);
	if (!HitEnemy || HitEnemy->IsDead())
	{
		return;
	}

	FDamageEvent DamageEvent;
	HitEnemy->TakeDamage(Damage, DamageEvent, InstigatorController, GetInstigator());

	UE_LOG(LogDiablo, Display, TEXT("Chain Lightning hit %s for %.0f damage (%d bounces left)"),
		*HitEnemy->GetName(), Damage, BouncesRemaining);

	if (BouncesRemaining > 0)
	{
		ADiabloEnemy* NextTarget = nullptr;
		float BestDist = BounceRange;

		TArray<FOverlapResult> Overlaps;
		FCollisionShape Shape = FCollisionShape::MakeSphere(BounceRange);
		GetWorld()->OverlapMultiByChannel(Overlaps, HitEnemy->GetActorLocation(),
			FQuat::Identity, ECC_Pawn, Shape);

		for (const FOverlapResult& Overlap : Overlaps)
		{
			ADiabloEnemy* Candidate = Cast<ADiabloEnemy>(Overlap.GetActor());
			if (Candidate && Candidate != HitEnemy && !Candidate->IsDead())
			{
				const float Dist = FVector::Dist(HitEnemy->GetActorLocation(),
					Candidate->GetActorLocation());
				if (Dist < BestDist)
				{
					BestDist = Dist;
					NextTarget = Candidate;
				}
			}
		}

		if (NextTarget)
		{
			const FVector Dir = (NextTarget->GetActorLocation() - HitEnemy->GetActorLocation()).GetSafeNormal();
			const FVector SpawnLoc = HitEnemy->GetActorLocation() + FVector(0.f, 0.f, 50.f);

			FActorSpawnParameters Params;
			Params.Instigator = Cast<APawn>(GetInstigator());
			Params.Owner = GetOwner();
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AChainLightning* Bounce = GetWorld()->SpawnActor<AChainLightning>(
				AChainLightning::StaticClass(), SpawnLoc, Dir.Rotation(), Params);
			if (Bounce)
			{
				Bounce->Damage = Damage;
				Bounce->BouncesRemaining = BouncesRemaining - 1;
				Bounce->BounceRange = BounceRange;
				Bounce->PreviousHitActor = HitEnemy;
			}
		}
	}

	Destroy();
}
