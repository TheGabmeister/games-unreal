#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class APlayerController;

/**
 * Phase 13 Slate widgets per DESIGN 6.3 / 6.4. Three single-use overlays —
 * death prompt, win screen, main menu — kept together because they share
 * the same poll-on-paint pattern and the same "centered text + click
 * targets" layout. Settings menu is its own widget below.
 *
 * All widgets are pure Slate (no UMG, no BP) to match the project's
 * "C++ first, no event-graph nodes" rule.
 */

// -----------------------------------------------------------------------------
// SQuakeDeathScreen — DESIGN 6.4. "You died — Press Fire to Restart."
// Visible only while the owning AQuakeCharacter::IsAwaitingRestart() is true.
// Input is consumed by AQuakePlayerController::OnFirePressedForRestart, not by
// the widget; the widget is purely informational.
// -----------------------------------------------------------------------------
class QUAKE_API SQuakeDeathScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuakeDeathScreen) {}
		SLATE_ARGUMENT(TWeakObjectPtr<APlayerController>, OwningPlayerController)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TWeakObjectPtr<APlayerController> OwningPlayerController;

	EVisibility GetDeathScreenVisibility() const;
	EVisibility GetPromptVisibility() const;
};

// -----------------------------------------------------------------------------
// SQuakeWinScreen — DESIGN 6.3. Total stats + "press fire to return to menu".
// Toggled on by AQuakeHUD::ShowWinScreen.
// -----------------------------------------------------------------------------
class QUAKE_API SQuakeWinScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuakeWinScreen) {}
		SLATE_ARGUMENT(TWeakObjectPtr<APlayerController>, OwningPlayerController)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetVisible(bool bVisible) { bShown = bVisible; }

private:
	TWeakObjectPtr<APlayerController> OwningPlayerController;
	bool bShown = false;

	EVisibility GetVisibility() const { return bShown ? EVisibility::HitTestInvisible : EVisibility::Collapsed; }

	FText GetKillsText() const;
	FText GetSecretsText() const;
	FText GetTimeText() const;
	FText GetDeathsText() const;
	FText GetDifficultyText() const;
};

// -----------------------------------------------------------------------------
// SQuakeMainMenu — used by editor-authored MainMenu.umap via AQuakeMenuHUD.
// New Game / Settings / Quit buttons. Difficulty selection happens inline:
// clicking "New Game" expands a difficulty picker, then opens the Hub map.
// -----------------------------------------------------------------------------
class QUAKE_API SQuakeMainMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuakeMainMenu) {}
		SLATE_ARGUMENT(TWeakObjectPtr<APlayerController>, OwningPlayerController)
		/** Map opened by "New Game" after difficulty selection. */
		SLATE_ARGUMENT(FName, HubMapName)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TWeakObjectPtr<APlayerController> OwningPlayerController;
	FName HubMapName;

	bool bShowingDifficultyPicker = false;
	bool bShowingSettings = false;

	EVisibility GetMainButtonsVisibility() const;
	EVisibility GetDifficultyPickerVisibility() const;
	EVisibility GetSettingsVisibility() const;

	FReply OnNewGameClicked();
	FReply OnSettingsClicked();
	FReply OnQuitClicked();
	FReply OnBackClicked();

	FReply OnDifficultyClicked(int32 Difficulty);
};

// -----------------------------------------------------------------------------
// SQuakeSettingsMenu — Phase 14 settings panel. Mouse sensitivity + master
// volume both read/write UQuakeGameUserSettings (persisted to
// GameUserSettings.ini on SaveSettings). Embeddable inside the main menu
// or as its own pause overlay.
// -----------------------------------------------------------------------------
class QUAKE_API SQuakeSettingsMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuakeSettingsMenu) {}
		SLATE_ARGUMENT(TWeakObjectPtr<APlayerController>, OwningPlayerController)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TWeakObjectPtr<APlayerController> OwningPlayerController;

	float CachedSensitivity = 0.5f;
	float CachedVolume = 1.f;

	float GetSensitivity() const { return CachedSensitivity; }
	void OnSensitivityChanged(float NewValue);
	FText GetSensitivityLabel() const;

	float GetVolume() const { return CachedVolume; }
	void OnVolumeChanged(float NewValue);
	FText GetVolumeLabel() const;
};
