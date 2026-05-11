#include "Projectiles/ProjectileBase.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CBPlayerCharacter.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->SetSphereRadius(20.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionComponent->SetGenerateOverlapEvents(true);
	SetRootComponent(CollisionComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileBase::InitDirection(FVector Direction)
{
	Velocity = Direction.GetSafeNormal() * Speed;
	SetLifeSpan(Lifetime);

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnOverlap);

	if (LaunchSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LaunchSound, GetActorLocation());
	}
}

void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ArcGravity > 0.0f)
	{
		Velocity.Z -= ArcGravity * DeltaTime;
	}

	FHitResult Hit;
	AddActorWorldOffset(Velocity * DeltaTime, true, &Hit);

	if (Hit.bBlockingHit)
	{
		OnImpact(Hit);
	}
}

void AProjectileBase::OnImpact(const FHitResult& Hit)
{
	Destroy();
}

void AProjectileBase::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Other == this || Other == GetInstigator()) return;

	if (ACBPlayerCharacter* Player = Cast<ACBPlayerCharacter>(Other))
	{
		if (!Player->IsDead())
		{
			Player->OnHit(this);
		}
		Destroy();
	}
}
