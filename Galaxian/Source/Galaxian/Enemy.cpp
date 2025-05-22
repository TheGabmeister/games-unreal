#include "Enemy.h"
#include "PaperSpriteComponent.h"

// Sets default values
AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	SpriteComp->PrimaryComponentTick.bStartWithTickEnabled = true;
	SpriteComp->SetGenerateOverlapEvents(true);
	SpriteComp->CanCharacterStepUpOn = ECB_No;
	SpriteComp->SetCollisionProfileName(TEXT("OverlapAll"));
}
