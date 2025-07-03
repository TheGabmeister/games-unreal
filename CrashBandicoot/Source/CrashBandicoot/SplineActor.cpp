#include "SplineActor.h"
#include "Components/SplineComponent.h"

// Sets default values
ASplineActor::ASplineActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComp"));
	SplineComp->SetupAttachment(RootComponent);
}

