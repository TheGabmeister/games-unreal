


#include "Enemy/CBEnemyPatrolRigActor.h"

#include "Enemy/CBEnemyPatrolRigComponent.h"

/*
* All we need to do here is create our default subobject, the CBEnemyPatrolRig, and set it as our root component.
* This means that the hierarchy is generated for each instance as we place it into the level.
*/
ACBEnemyPatrolRigActor::ACBEnemyPatrolRigActor()
{
	PatrolRigComponent = CreateDefaultSubobject<UCBEnemyPatrolRigComponent>(TEXT("PatrolRig"));
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	PatrolRigComponent->SetupAttachment(RootComponent);
}
