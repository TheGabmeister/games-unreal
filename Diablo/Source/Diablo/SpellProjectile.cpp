#include "SpellProjectile.h"
#include "DiabloHero.h"
#include "DiabloEnemy.h"
#include "Diablo.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/DamageEvents.h"

ASpellProjectile::ASpellProjectile()
{
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->InitSphereRadius(20.f);
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = CollisionComponent;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	InitialLifeSpan = 5.f;
}

void ASpellProjectile::BeginPlay()
{
	Super::BeginPlay();

	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->Velocity = GetActorForwardVector() * Speed;

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASpellProjectile::OnOverlap);

	if (APawn* OwnerPawn = Cast<APawn>(GetInstigator()))
	{
		InstigatorController = OwnerPawn->GetController();
	}
}

void ASpellProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetInstigator())
	{
		return;
	}

	if (bDamageEnemies)
	{
		ADiabloEnemy* Enemy = Cast<ADiabloEnemy>(OtherActor);
		if (Enemy && !Enemy->IsDead())
		{
			FDamageEvent DamageEvent;
			Enemy->TakeDamage(Damage, DamageEvent, InstigatorController, GetInstigator());

			UE_LOG(LogDiablo, Display, TEXT("%s hit %s for %.0f damage"),
				*GetName(), *Enemy->GetName(), Damage);

			Destroy();
			return;
		}
	}

	if (bDamageHero)
	{
		ADiabloHero* Hero = Cast<ADiabloHero>(OtherActor);
		if (Hero && !Hero->IsDead())
		{
			FDamageEvent DamageEvent;
			Hero->TakeDamage(Damage, DamageEvent, InstigatorController, GetInstigator());

			UE_LOG(LogDiablo, Display, TEXT("%s hit %s for %.0f damage"),
				*GetName(), *Hero->GetName(), Damage);

			Destroy();
		}
	}
}
