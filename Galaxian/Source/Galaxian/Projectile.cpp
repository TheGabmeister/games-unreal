#include "Projectile.h"
#include "PaperSpriteComponent.h"

// Sets default values
AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	SpriteComp->PrimaryComponentTick.bStartWithTickEnabled = true;
	SpriteComp->SetGenerateOverlapEvents(true);
	SpriteComp->CanCharacterStepUpOn = ECB_No;
	SpriteComp->SetCollisionProfileName(TEXT("NoCollision"));

	SpriteComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnOverlap);
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AddActorWorldOffset(Velocity * DeltaTime, true);
}

void AProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Destroy();
}
