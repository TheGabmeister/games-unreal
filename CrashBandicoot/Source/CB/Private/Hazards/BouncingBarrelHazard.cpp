#include "Hazards/BouncingBarrelHazard.h"

ABouncingBarrelHazard::ABouncingBarrelHazard()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABouncingBarrelHazard::BeginPlay()
{
	Super::BeginPlay();
	GroundLocation = GetActorLocation();
}

void ABouncingBarrelHazard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeAccumulator += DeltaTime;

	float SinValue = FMath::Abs(FMath::Sin(TimeAccumulator * BounceFrequency * PI));
	float HeightOffset = SinValue * BounceHeight;
	SetActorLocation(GroundLocation + FVector(0.0f, 0.0f, HeightOffset));
}
