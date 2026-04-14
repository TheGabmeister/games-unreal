#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class APlayerController;
class AQuakeCharacter;
class AQuakeGameMode;
class AQuakePlayerState;

/**
 * Phase 2 minimal HUD overlay. Pure Slate (no UMG, no BP) per SPEC
 * section 7. Polls the player pawn on paint via its owning controller —
 * the controller survives pawn replacement (death / respawn, future
 * teleport) whereas a pawn pointer would go stale the moment the body
 * dies.
 *
 * Phase 9 additions: top-left stats strip (Kills X/N, Secrets X/N, Time
 * mm:ss) and an end-of-level stats screen toggled via ShowLevelEndStats.
 * All values are pulled fresh from AQuakePlayerState (numerators + time)
 * and AQuakeGameMode (denominators) on every paint so the widget is
 * stateless and no explicit refresh is needed.
 */
class QUAKE_API SQuakeHUDOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuakeHUDOverlay) {}
		SLATE_ARGUMENT(TWeakObjectPtr<APlayerController>, OwningPlayerController)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/**
	 * Show the level-end stats screen for Duration seconds. Called from
	 * AQuakeHUD::ShowLevelEndStats (which the exit trigger invokes).
	 */
	void ShowLevelEndStats(float Duration);

private:
	TWeakObjectPtr<APlayerController> OwningPlayerController;

	/** World time at which the level-end stats screen stops displaying. */
	double LevelEndStatsExpireWorldTime = 0.0;

	/** Resolves the currently-possessed AQuakeCharacter, or nullptr. */
	const AQuakeCharacter* ResolvePlayerCharacter() const;
	AQuakePlayerState* ResolvePlayerState() const;
	const AQuakeGameMode* ResolveGameMode() const;

	FText GetHealthText() const;
	FText GetWeaponText() const;
	FText GetAmmoText() const;

	// Phase 9 stats strip.
	FText GetKillsText() const;
	FText GetSecretsText() const;
	FText GetTimeText() const;

	// Phase 10 HUD additions.
	FText GetArmorText() const;
	FSlateColor GetArmorColor() const;
	EVisibility GetArmorVisibility() const;
	FText GetSilverKeyText() const;
	EVisibility GetSilverKeyVisibility() const;
	FText GetGoldKeyText() const;
	EVisibility GetGoldKeyVisibility() const;
	FText GetQuadTimerText() const;
	EVisibility GetQuadTimerVisibility() const;

	// Phase 9 level-end stats screen.
	EVisibility GetLevelEndStatsVisibility() const;
	FText GetLevelEndTitleText() const;
	FText GetLevelEndKillsText() const;
	FText GetLevelEndSecretsText() const;
	FText GetLevelEndTimeText() const;
	FText GetLevelEndDeathsText() const;
};
