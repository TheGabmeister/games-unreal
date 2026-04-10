#include "QuakePlayerState.h"

AQuakePlayerState::AQuakePlayerState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AQuakePlayerState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TimeElapsed += DeltaSeconds;

	for (int32 i = ActivePowerups.Num() - 1; i >= 0; --i)
	{
		ActivePowerups[i].RemainingTime -= DeltaSeconds;
		if (ActivePowerups[i].RemainingTime <= 0.f)
		{
			ActivePowerups.RemoveAt(i);
		}
	}
}
