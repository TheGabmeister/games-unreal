#include "Enemy/LaunchedEnemyProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Enemy/CBEnemyCharacterBase.h"
#include "Interfaces/InteractionInterfaces.h"
#include "Player/CBPlayerCharacter.h"

ALaunchedEnemyProjectile::ALaunchedEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(40.0f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetGenerateOverlapEvents(true);
	SetRootComponent(CollisionSphere);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(CollisionSphere);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InitialLifeSpan = 3.0f;
}

void ALaunchedEnemyProjectile::InitProjectile(FVector Direction, float Speed)
{
	Velocity = Direction.GetSafeNormal() * Speed;
	SetLifeSpan(Lifetime);

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ALaunchedEnemyProjectile::OnProjectileOverlap);
}

void ALaunchedEnemyProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Velocity.Z -= 980.0f * GravityScale * DeltaTime;

	FHitResult Hit;
	AddActorWorldOffset(Velocity * DeltaTime, true, &Hit);

	if (Hit.bBlockingHit)
	{
		Destroy();
		return;
	}

	// Tumble rotation for visual effect
	MeshComponent->AddRelativeRotation(FRotator(360.0f * DeltaTime, 180.0f * DeltaTime, 0.0f));
}

void ALaunchedEnemyProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Other == this) return;

	// Skip player — launched enemies don't hurt the player
	if (Cast<ACBPlayerCharacter>(Other)) return;

	if (IExplodable* Explodable = Cast<IExplodable>(Other))
	{
		Explodable->OnExplosionHit(GetActorLocation(), 0.0f);
	}

	Destroy();
}
