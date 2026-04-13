#include "QuakeTrigger_Exit.h"

#include "QuakeGameMode.h"
#include "QuakeHUD.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeExit, Log, All);

AQuakeTrigger_Exit::AQuakeTrigger_Exit() = default;

void AQuakeTrigger_Exit::Activate(AActor* InInstigator)
{
	if (bTransitionInFlight)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// SPEC 5.9: refuse exit while the level isn't cleared. Log + HUD
	// message so the player understands why the walk-through did nothing.
	if (bGatedByClearCondition)
	{
		const AQuakeGameMode* GM = World->GetAuthGameMode<AQuakeGameMode>();
		if (GM && !GM->IsLevelCleared())
		{
			UE_LOG(LogQuakeExit, Log, TEXT("%s: exit gated — level not cleared."), *GetName());
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
			{
				if (AQuakeHUD* HUD = Cast<AQuakeHUD>(PC->GetHUD()))
				{
					HUD->ShowMessage(
						NSLOCTEXT("Quake", "ExitGated", "All enemies must be cleared before exiting."),
						2.f);
				}
			}
			return;
		}
	}

	// Fire the base Targets chain first so on-exit bookkeeping (messages,
	// relays) runs while the level is still live.
	Super::Activate(InInstigator);

	bTransitionInFlight = true;

	// SPEC 11.5 Phase 9: show end-of-level stats for ~5 s, then transition.
	// The HUD's stats screen polls PlayerState + GameMode directly; we just
	// flip the "show" flag.
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (AQuakeHUD* HUD = Cast<AQuakeHUD>(PC->GetHUD()))
		{
			HUD->ShowLevelEndStats(StatsDisplaySeconds);
		}
	}

	if (NextMapName.IsNone())
	{
		UE_LOG(LogQuakeExit, Warning,
			TEXT("%s: NextMapName is None — stats screen will show, no transition."), *GetName());
		return;
	}

	World->GetTimerManager().SetTimer(
		TransitionTimer, this, &AQuakeTrigger_Exit::OnStatsScreenTimeout,
		StatsDisplaySeconds, /*bLoop*/ false);
}

void AQuakeTrigger_Exit::OnStatsScreenTimeout()
{
	UE_LOG(LogQuakeExit, Log, TEXT("%s: exiting to %s"), *GetName(), *NextMapName.ToString());
	UGameplayStatics::OpenLevel(this, NextMapName);
}
