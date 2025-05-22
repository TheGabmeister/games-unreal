#include "Enemy.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"

// Sets default values
AEnemy::AEnemy()
{
	//Super::ABaseActor2D();
	PrimaryActorTick.bCanEverTick = false;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
    BoxComp->SetupAttachment(SpriteComp);
}
