#include "BaseActor2D.h"
#include "PaperSpriteComponent.h"

ABaseActor2D::ABaseActor2D()
{
	PrimaryActorTick.bCanEverTick = false;

	SpriteComp = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComp"));
	SpriteComp->SetupAttachment(RootComponent);

	SpriteComp->PrimaryComponentTick.bStartWithTickEnabled = false;
	SpriteComp->bEnableAutoLODGeneration = 0;
	SpriteComp->SetEnableGravity(false);
	SpriteComp->SetGenerateOverlapEvents(false);
	SpriteComp->CanCharacterStepUpOn = ECB_No; 
	SpriteComp->SetCollisionProfileName(TEXT("NoCollision"));
}
