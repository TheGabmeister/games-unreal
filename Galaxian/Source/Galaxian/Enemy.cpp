#include "Enemy.h"
#include "PaperSpriteComponent.h"

// Sets default values
AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	SpriteComp->PrimaryComponentTick.bStartWithTickEnabled = true;
	SpriteComp->SetGenerateOverlapEvents(true);
	SpriteComp->CanCharacterStepUpOn = ECB_No;
	SpriteComp->SetCollisionProfileName(TEXT("Enemy"));

	SpriteComp->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnOverlap);
}

void AEnemy::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
					   int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Destroy();
}
