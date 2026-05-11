#include "Hazards/FlyingFishHazard.h"

#include "Components/SphereComponent.h"

AFlyingFishHazard::AFlyingFishHazard()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFlyingFishHazard::BeginPlay()
{
	Super::BeginPlay();
	RestLocation = GetActorLocation();

	SetActorHiddenInGame(true);
	DamageVolume->SetGenerateOverlapEvents(false);
}

void AFlyingFishHazard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Timer += DeltaTime;

	if (!bJumping)
	{
		if (Timer >= RestDuration)
		{
			bJumping = true;
			Timer = 0.0f;
			SetActorHiddenInGame(false);
			DamageVolume->SetGenerateOverlapEvents(true);
		}
	}
	else
	{
		float Alpha = Timer / JumpDuration;

		if (Alpha >= 1.0f)
		{
			bJumping = false;
			Timer = 0.0f;
			SetActorLocation(RestLocation);
			SetActorHiddenInGame(true);
			DamageVolume->SetGenerateOverlapEvents(false);
			return;
		}

		// Parabolic arc: 4 * h * t * (1-t) where t = alpha
		float HeightOffset = 4.0f * JumpHeight * Alpha * (1.0f - Alpha);
		SetActorLocation(RestLocation + FVector(0.0f, 0.0f, HeightOffset));
	}
}
