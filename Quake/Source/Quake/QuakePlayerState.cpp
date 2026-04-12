#include "QuakePlayerState.h"

#include "Engine/World.h"

AQuakePlayerState::AQuakePlayerState()
{
	// Tick is off by default — only enabled while powerups are active.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AQuakePlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (const UWorld* World = GetWorld())
	{
		LevelStartTime = World->GetTimeSeconds();
	}
}

float AQuakePlayerState::GetTimeElapsed() const
{
	const UWorld* World = GetWorld();
	return World ? static_cast<float>(World->GetTimeSeconds() - LevelStartTime) : 0.f;
}

void AQuakePlayerState::EnablePowerupTick()
{
	SetActorTickEnabled(true);
}

void AQuakePlayerState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	for (int32 i = ActivePowerups.Num() - 1; i >= 0; --i)
	{
		ActivePowerups[i].RemainingTime -= DeltaSeconds;
		if (ActivePowerups[i].RemainingTime <= 0.f)
		{
			ActivePowerups.RemoveAt(i);
		}
	}

	// No active powerups left — stop ticking until the next grant.
	if (ActivePowerups.Num() == 0)
	{
		SetActorTickEnabled(false);
	}
}
