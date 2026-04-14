#include "QuakePlayerState.h"

#include "QuakeSaveArchive.h"
#include "QuakeSaveGame.h"

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

void AQuakePlayerState::GivePowerup(EQuakePowerup Type, float Duration)
{
	if (Type == EQuakePowerup::None || Duration <= 0.f)
	{
		return;
	}

	const float Cap = GetPowerupMaxDuration();

	for (FQuakeActivePowerup& Entry : ActivePowerups)
	{
		if (Entry.Type == Type)
		{
			// SPEC 4.3: additive refresh capped at 60 s.
			Entry.RemainingTime = FMath::Min(Entry.RemainingTime + Duration, Cap);
			return;
		}
	}

	FQuakeActivePowerup NewEntry;
	NewEntry.Type = Type;
	NewEntry.RemainingTime = FMath::Min(Duration, Cap);
	ActivePowerups.Add(NewEntry);
	EnablePowerupTick();
}

bool AQuakePlayerState::HasPowerup(EQuakePowerup Type) const
{
	if (Type == EQuakePowerup::None)
	{
		return false;
	}
	for (const FQuakeActivePowerup& Entry : ActivePowerups)
	{
		if (Entry.Type == Type && Entry.RemainingTime > 0.f)
		{
			return true;
		}
	}
	return false;
}

float AQuakePlayerState::GetPowerupRemaining(EQuakePowerup Type) const
{
	for (const FQuakeActivePowerup& Entry : ActivePowerups)
	{
		if (Entry.Type == Type)
		{
			return Entry.RemainingTime;
		}
	}
	return 0.f;
}

bool AQuakePlayerState::HasKey(EQuakeKeyColor Color) const
{
	return Color != EQuakeKeyColor::None && Keys.Contains(Color);
}

void AQuakePlayerState::GiveKey(EQuakeKeyColor Color)
{
	if (Color == EQuakeKeyColor::None)
	{
		return;
	}
	Keys.AddUnique(Color);
}

void AQuakePlayerState::ClearPerLifeState()
{
	ActivePowerups.Empty();
	Keys.Empty();
	SetActorTickEnabled(false);
}

void AQuakePlayerState::CaptureToSave(UQuakeSaveGame& Out) const
{
	Out.Kills          = Kills;
	Out.Secrets        = Secrets;
	Out.Deaths         = Deaths;
	Out.ElapsedAtSave  = GetTimeElapsed();
	Out.ActivePowerups = ActivePowerups;
	Out.Keys           = Keys;
}

void AQuakePlayerState::ApplyFromSave(const UQuakeSaveGame& In, double WorldTimeNow)
{
	Kills          = In.Kills;
	Secrets        = In.Secrets;
	Deaths         = In.Deaths;
	ActivePowerups = In.ActivePowerups;
	Keys           = In.Keys;

	// DESIGN 6.2: translate the saved elapsed count back into a LevelStartTime
	// so GetTimeElapsed() resumes at the saved value against the new world's
	// clock.
	LevelStartTime = QuakeSaveArchive::ComputeRestoredLevelStartTime(WorldTimeNow, In.ElapsedAtSave);

	if (ActivePowerups.Num() > 0)
	{
		EnablePowerupTick();
	}
	else
	{
		SetActorTickEnabled(false);
	}
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
