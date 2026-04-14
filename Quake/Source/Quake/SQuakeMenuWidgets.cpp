#include "SQuakeMenuWidgets.h"

#include "QuakeCharacter.h"
#include "QuakeDifficulty.h"
#include "QuakeGameInstance.h"
#include "QuakeGameMode.h"
#include "QuakeGameUserSettings.h"
#include "QuakePlayerState.h"

#include "Engine/World.h"
#include "Fonts/SlateFontInfo.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "QuakeMenu"

namespace
{
	// Format mm:ss for time-elapsed displays.
	FText FormatTime(float TimeSeconds)
	{
		const int32 Total = FMath::Max(0, FMath::FloorToInt(TimeSeconds));
		const int32 Mins  = Total / 60;
		const int32 Secs  = Total % 60;
		return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Mins, Secs));
	}

	FText DifficultyToText(EQuakeDifficulty Difficulty)
	{
		switch (Difficulty)
		{
		case EQuakeDifficulty::Easy:      return LOCTEXT("DiffEasy",      "Easy");
		case EQuakeDifficulty::Normal:    return LOCTEXT("DiffNormal",    "Normal");
		case EQuakeDifficulty::Hard:      return LOCTEXT("DiffHard",      "Hard");
		case EQuakeDifficulty::Nightmare: return LOCTEXT("DiffNightmare", "Nightmare");
		}
		return FText::GetEmpty();
	}

	FSlateFontInfo TitleFont()  { return FCoreStyle::GetDefaultFontStyle("Bold",    48); }
	FSlateFontInfo BodyFont()   { return FCoreStyle::GetDefaultFontStyle("Bold",    24); }
	FSlateFontInfo PromptFont() { return FCoreStyle::GetDefaultFontStyle("Regular", 18); }
}

// -----------------------------------------------------------------------------
// SQuakeDeathScreen
// -----------------------------------------------------------------------------
void SQuakeDeathScreen::Construct(const FArguments& InArgs)
{
	OwningPlayerController = InArgs._OwningPlayerController;

	ChildSlot
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.6f))
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Visibility(this, &SQuakeDeathScreen::GetDeathScreenVisibility)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(0.4f)[ SNew(SBox) ]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(TitleFont())
				.ColorAndOpacity(FLinearColor(0.9f, 0.2f, 0.2f))
				.Text(LOCTEXT("YouDied", "You died."))
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 32.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Font(BodyFont())
				.ColorAndOpacity(FLinearColor::White)
				.Text(LOCTEXT("PressFireRestart", "Press Fire to Restart"))
				.Visibility(this, &SQuakeDeathScreen::GetPromptVisibility)
			]
			+ SVerticalBox::Slot().FillHeight(0.6f)[ SNew(SBox) ]
		]
	];
}

EVisibility SQuakeDeathScreen::GetDeathScreenVisibility() const
{
	const APlayerController* PC = OwningPlayerController.Get();
	const AQuakeCharacter* Char = PC ? Cast<AQuakeCharacter>(PC->GetPawn()) : nullptr;
	return (Char && Char->IsAwaitingRestart())
		? EVisibility::HitTestInvisible
		: EVisibility::Collapsed;
}

EVisibility SQuakeDeathScreen::GetPromptVisibility() const
{
	const APlayerController* PC = OwningPlayerController.Get();
	const AQuakeCharacter* Char = PC ? Cast<AQuakeCharacter>(PC->GetPawn()) : nullptr;
	if (!Char || !Char->IsAwaitingRestart()) return EVisibility::Collapsed;

	const UWorld* World = PC->GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;
	return (Now >= Char->GetRestartReadyWorldTime())
		? EVisibility::HitTestInvisible
		: EVisibility::Hidden;
}

// -----------------------------------------------------------------------------
// SQuakeWinScreen
// -----------------------------------------------------------------------------
void SQuakeWinScreen::Construct(const FArguments& InArgs)
{
	OwningPlayerController = InArgs._OwningPlayerController;

	ChildSlot
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.85f))
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Visibility(this, &SQuakeWinScreen::GetVisibility)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(0.3f)[ SNew(SBox) ]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(TitleFont())
				.ColorAndOpacity(FLinearColor(0.9f, 0.8f, 0.2f))
				.Text(LOCTEXT("WinTitle", "Episode Complete"))
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 40.f, 0.f, 0.f)
			[
				SNew(STextBlock).Font(BodyFont()).ColorAndOpacity(FLinearColor::White)
				.Text(this, &SQuakeWinScreen::GetKillsText)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock).Font(BodyFont()).ColorAndOpacity(FLinearColor::White)
				.Text(this, &SQuakeWinScreen::GetSecretsText)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock).Font(BodyFont()).ColorAndOpacity(FLinearColor::White)
				.Text(this, &SQuakeWinScreen::GetTimeText)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock).Font(BodyFont()).ColorAndOpacity(FLinearColor::White)
				.Text(this, &SQuakeWinScreen::GetDeathsText)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock).Font(BodyFont()).ColorAndOpacity(FLinearColor::White)
				.Text(this, &SQuakeWinScreen::GetDifficultyText)
			]
			+ SVerticalBox::Slot().FillHeight(0.7f)[ SNew(SBox) ]
		]
	];
}

FText SQuakeWinScreen::GetKillsText() const
{
	const APlayerController* PC = OwningPlayerController.Get();
	const AQuakePlayerState* PS = PC ? PC->GetPlayerState<AQuakePlayerState>() : nullptr;
	const int32 K = PS ? PS->Kills : 0;
	return FText::Format(LOCTEXT("WinKills", "Kills: {0}"), K);
}

FText SQuakeWinScreen::GetSecretsText() const
{
	const APlayerController* PC = OwningPlayerController.Get();
	const AQuakePlayerState* PS = PC ? PC->GetPlayerState<AQuakePlayerState>() : nullptr;
	const int32 S = PS ? PS->Secrets : 0;
	return FText::Format(LOCTEXT("WinSecrets", "Secrets: {0}"), S);
}

FText SQuakeWinScreen::GetTimeText() const
{
	const APlayerController* PC = OwningPlayerController.Get();
	const AQuakePlayerState* PS = PC ? PC->GetPlayerState<AQuakePlayerState>() : nullptr;
	const float T = PS ? PS->GetTimeElapsed() : 0.f;
	return FText::Format(LOCTEXT("WinTime", "Time: {0}"), FormatTime(T));
}

FText SQuakeWinScreen::GetDeathsText() const
{
	const APlayerController* PC = OwningPlayerController.Get();
	const AQuakePlayerState* PS = PC ? PC->GetPlayerState<AQuakePlayerState>() : nullptr;
	const int32 D = PS ? PS->Deaths : 0;
	return FText::Format(LOCTEXT("WinDeaths", "Deaths: {0}"), D);
}

FText SQuakeWinScreen::GetDifficultyText() const
{
	const APlayerController* PC = OwningPlayerController.Get();
	const UWorld* World = PC ? PC->GetWorld() : nullptr;
	const UQuakeGameInstance* GI = World ? World->GetGameInstance<UQuakeGameInstance>() : nullptr;
	const EQuakeDifficulty D = GI ? GI->GetDifficulty() : EQuakeDifficulty::Normal;
	return FText::Format(LOCTEXT("WinDifficulty", "Difficulty: {0}"), DifficultyToText(D));
}

// -----------------------------------------------------------------------------
// SQuakeMainMenu
// -----------------------------------------------------------------------------
void SQuakeMainMenu::Construct(const FArguments& InArgs)
{
	OwningPlayerController = InArgs._OwningPlayerController;
	HubMapName = InArgs._HubMapName;

	ChildSlot
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.95f))
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SOverlay)

			// Title (always visible).
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 80.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Font(TitleFont())
				.ColorAndOpacity(FLinearColor(0.9f, 0.2f, 0.2f))
				.Text(LOCTEXT("GameTitle", "QUAKE"))
			]

			// Main buttons.
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				.Visibility(this, &SQuakeMainMenu::GetMainButtonsVisibility)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("BtnNewGame", "  New Game  "))
					.OnClicked(this, &SQuakeMainMenu::OnNewGameClicked)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("BtnSettings", "  Settings  "))
					.OnClicked(this, &SQuakeMainMenu::OnSettingsClicked)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("BtnQuit", "  Quit  "))
					.OnClicked(this, &SQuakeMainMenu::OnQuitClicked)
				]
			]

			// Difficulty picker (shown after New Game).
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				.Visibility(this, &SQuakeMainMenu::GetDifficultyPickerVisibility)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 24.f)
				[
					SNew(STextBlock).Font(BodyFont()).ColorAndOpacity(FLinearColor::White)
					.Text(LOCTEXT("PickDifficulty", "Choose Difficulty"))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)
				[
					SNew(SButton).Text(DifficultyToText(EQuakeDifficulty::Easy))
					.OnClicked(this, &SQuakeMainMenu::OnDifficultyClicked,
						static_cast<int32>(EQuakeDifficulty::Easy))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)
				[
					SNew(SButton).Text(DifficultyToText(EQuakeDifficulty::Normal))
					.OnClicked(this, &SQuakeMainMenu::OnDifficultyClicked,
						static_cast<int32>(EQuakeDifficulty::Normal))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)
				[
					SNew(SButton).Text(DifficultyToText(EQuakeDifficulty::Hard))
					.OnClicked(this, &SQuakeMainMenu::OnDifficultyClicked,
						static_cast<int32>(EQuakeDifficulty::Hard))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)
				[
					SNew(SButton).Text(DifficultyToText(EQuakeDifficulty::Nightmare))
					.OnClicked(this, &SQuakeMainMenu::OnDifficultyClicked,
						static_cast<int32>(EQuakeDifficulty::Nightmare))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
				[
					SNew(SButton).Text(LOCTEXT("BtnBack", "  Back  "))
					.OnClicked(this, &SQuakeMainMenu::OnBackClicked)
				]
			]

			// Settings panel (shown after Settings).
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				.Visibility(this, &SQuakeMainMenu::GetSettingsVisibility)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SQuakeSettingsMenu).OwningPlayerController(OwningPlayerController)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 16.f, 0.f, 0.f)
				[
					SNew(SButton).Text(LOCTEXT("BtnBack", "  Back  "))
					.OnClicked(this, &SQuakeMainMenu::OnBackClicked)
				]
			]
		]
	];
}

EVisibility SQuakeMainMenu::GetMainButtonsVisibility() const
{
	return (!bShowingDifficultyPicker && !bShowingSettings)
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

EVisibility SQuakeMainMenu::GetDifficultyPickerVisibility() const
{
	return bShowingDifficultyPicker ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SQuakeMainMenu::GetSettingsVisibility() const
{
	return bShowingSettings ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SQuakeMainMenu::OnNewGameClicked()
{
	bShowingDifficultyPicker = true;
	return FReply::Handled();
}

FReply SQuakeMainMenu::OnSettingsClicked()
{
	bShowingSettings = true;
	return FReply::Handled();
}

FReply SQuakeMainMenu::OnQuitClicked()
{
	if (APlayerController* PC = OwningPlayerController.Get())
	{
		UKismetSystemLibrary::QuitGame(PC, PC, EQuitPreference::Quit, /*bIgnorePlatformRestrictions*/ false);
	}
	return FReply::Handled();
}

FReply SQuakeMainMenu::OnBackClicked()
{
	bShowingDifficultyPicker = false;
	bShowingSettings = false;
	return FReply::Handled();
}

FReply SQuakeMainMenu::OnDifficultyClicked(int32 Difficulty)
{
	APlayerController* PC = OwningPlayerController.Get();
	UWorld* World = PC ? PC->GetWorld() : nullptr;
	if (!World) return FReply::Handled();

	if (UQuakeGameInstance* GI = World->GetGameInstance<UQuakeGameInstance>())
	{
		GI->SetDifficulty(static_cast<EQuakeDifficulty>(Difficulty));
	}

	if (!HubMapName.IsNone())
	{
		UGameplayStatics::OpenLevel(World, HubMapName);
	}
	return FReply::Handled();
}

// -----------------------------------------------------------------------------
// SQuakeSettingsMenu
// -----------------------------------------------------------------------------
void SQuakeSettingsMenu::Construct(const FArguments& InArgs)
{
	OwningPlayerController = InArgs._OwningPlayerController;

	// Phase 14: settings live in UQuakeGameUserSettings (persisted via
	// SaveSettings -> GameUserSettings.ini). Fall back to the live pawn's
	// LookSensitivity if the user-settings class isn't wired in (project
	// missing the GameUserSettingsClassName ini line).
	if (UQuakeGameUserSettings* Settings = UQuakeGameUserSettings::Get())
	{
		CachedSensitivity = Settings->GetMouseSensitivity();
		CachedVolume = Settings->GetMasterVolume();
	}
	else if (const APlayerController* PC = OwningPlayerController.Get())
	{
		if (const AQuakeCharacter* Char = Cast<AQuakeCharacter>(PC->GetPawn()))
		{
			CachedSensitivity = Char->LookSensitivity;
		}
	}

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 16.f)
		[
			SNew(STextBlock).Font(BodyFont()).ColorAndOpacity(FLinearColor::White)
			.Text(LOCTEXT("SettingsTitle", "Settings"))
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(STextBlock).Font(PromptFont()).ColorAndOpacity(FLinearColor::White)
			.Text(this, &SQuakeSettingsMenu::GetSensitivityLabel)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(8.f)
		[
			SNew(SBox).WidthOverride(320.f)
			[
				SNew(SSlider)
				.MinValue(0.05f)
				.MaxValue(2.0f)
				.Value(this, &SQuakeSettingsMenu::GetSensitivity)
				.OnValueChanged(this, &SQuakeSettingsMenu::OnSensitivityChanged)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 16.f, 0.f, 0.f)
		[
			SNew(STextBlock).Font(PromptFont()).ColorAndOpacity(FLinearColor::White)
			.Text(this, &SQuakeSettingsMenu::GetVolumeLabel)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(8.f)
		[
			SNew(SBox).WidthOverride(320.f)
			[
				SNew(SSlider)
				.MinValue(0.f)
				.MaxValue(1.f)
				.Value(this, &SQuakeSettingsMenu::GetVolume)
				.OnValueChanged(this, &SQuakeSettingsMenu::OnVolumeChanged)
			]
		]
	];
}

void SQuakeSettingsMenu::OnSensitivityChanged(float NewValue)
{
	CachedSensitivity = NewValue;

	if (UQuakeGameUserSettings* Settings = UQuakeGameUserSettings::Get())
	{
		Settings->SetMouseSensitivity(NewValue);
		Settings->SaveSettings();
	}

	// Apply immediately to the live pawn so the change is visible without
	// requiring a level reload — the pawn reads LookSensitivity per Look tick.
	if (APlayerController* PC = OwningPlayerController.Get())
	{
		if (AQuakeCharacter* Char = Cast<AQuakeCharacter>(PC->GetPawn()))
		{
			Char->LookSensitivity = NewValue;
		}
	}
}

FText SQuakeSettingsMenu::GetSensitivityLabel() const
{
	return FText::Format(LOCTEXT("SensLabel", "Mouse Sensitivity: {0}"),
		FText::AsNumber(CachedSensitivity));
}

void SQuakeSettingsMenu::OnVolumeChanged(float NewValue)
{
	CachedVolume = NewValue;
	if (UQuakeGameUserSettings* Settings = UQuakeGameUserSettings::Get())
	{
		Settings->SetMasterVolume(NewValue);
		Settings->SaveSettings();
	}
	// No audio routing yet — Phase 14 scope is "settings persist". Hooking
	// the value to a USoundMix happens once the first real sound asset lands.
}

FText SQuakeSettingsMenu::GetVolumeLabel() const
{
	return FText::Format(LOCTEXT("VolumeLabel", "Master Volume: {0}"),
		FText::AsNumber(CachedVolume));
}

#undef LOCTEXT_NAMESPACE
