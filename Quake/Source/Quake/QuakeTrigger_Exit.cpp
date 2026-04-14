#include "QuakeTrigger_Exit.h"

#include "QuakeCharacter.h"
#include "QuakeGameInstance.h"
#include "QuakeGameMode.h"
#include "QuakeHUD.h"
#include "QuakeInventoryComponent.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeExit, Log, All);

namespace
{
	/**
	 * Serialize the live pawn's inventory into GI->TransitSnapshot just
	 * before OpenLevel. The new world's Character spawns, and its
	 * InventoryComponent::InitializeComponent consumes this mailbox.
	 * Without this handoff, the next pawn starts with UPROPERTY defaults
	 * (starting loadout) — which is what we want for new-game flow but
	 * not for level-to-level transitions.
	 */
	void HandoffInventoryToNextLevel(UWorld* World)
	{
		APlayerController* PC = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
		const AQuakeCharacter* Char = PC ? Cast<AQuakeCharacter>(PC->GetPawn()) : nullptr;
		const UQuakeInventoryComponent* Inv = Char ? Char->GetInventoryComponent() : nullptr;
		UQuakeGameInstance* GI = UQuakeGameInstance::GetChecked(World);
		if (Inv)
		{
			Inv->SerializeTo(GI->TransitSnapshot);
		}
		else
		{
			GI->TransitSnapshot.bValid = false;
		}
	}
}

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

	// DESIGN 6.3: final-level exits route to the win screen instead of
	// OpenLevel. The HUD widget polls bWinScreenActive on AQuakeHUD.
	const AQuakeGameMode* GM = World->GetAuthGameMode<AQuakeGameMode>();
	const bool bWin = GM && AQuakeGameMode::ShouldRouteToWinScreen(GM->bIsFinalLevel, NextMapName);
	if (bWin)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			if (AQuakeHUD* HUD = Cast<AQuakeHUD>(PC->GetHUD()))
			{
				HUD->ShowWinScreen();
			}
		}
		// Schedule return-to-menu on the same display window. Empty
		// MainMenuMapName falls through to a log + no transition (manual
		// return-to-menu via the win-screen widget will exist once the
		// menu map asset is authored).
		if (GM && !GM->MainMenuMapName.IsNone())
		{
			World->GetTimerManager().SetTimer(
				TransitionTimer, this, &AQuakeTrigger_Exit::OnStatsScreenTimeout,
				StatsDisplaySeconds, /*bLoop*/ false);
		}
		return;
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
	UWorld* World = GetWorld();
	const AQuakeGameMode* GM = World ? World->GetAuthGameMode<AQuakeGameMode>() : nullptr;
	if (GM && GM->bIsFinalLevel && !GM->MainMenuMapName.IsNone())
	{
		UE_LOG(LogQuakeExit, Log, TEXT("%s: win → %s"), *GetName(), *GM->MainMenuMapName.ToString());
		// Win-to-menu: no handoff. The menu map runs with default inventory,
		// and a "new game" press wipes everything anyway.
		UGameplayStatics::OpenLevel(this, GM->MainMenuMapName);
		return;
	}
	UE_LOG(LogQuakeExit, Log, TEXT("%s: exiting to %s"), *GetName(), *NextMapName.ToString());
	// Hand off live inventory to the next level via GI->TransitSnapshot.
	HandoffInventoryToNextLevel(World);
	UGameplayStatics::OpenLevel(this, NextMapName);
}
