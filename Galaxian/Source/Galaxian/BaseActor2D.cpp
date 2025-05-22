#include "BaseActor2D.h"
#include "PaperSpriteComponent.h"

ABaseActor2D::ABaseActor2D()
{
	PrimaryActorTick.bCanEverTick = false;

	SpriteComp = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComp"));
	SpriteComp->SetupAttachment(RootComponent);
}
