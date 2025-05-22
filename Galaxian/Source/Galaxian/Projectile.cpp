#include "Projectile.h"
#include "PaperSpriteComponent.h"

// Sets default values
AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	SpriteComp->PrimaryComponentTick.bStartWithTickEnabled = true;
	SpriteComp->SetGenerateOverlapEvents(true);
	SpriteComp->CanCharacterStepUpOn = ECB_No;
	SpriteComp->SetCollisionProfileName(TEXT("Custom"));
	SpriteComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SpriteComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SpriteComp->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);

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

