#include "SQuakeHUDOverlay.h"

#include "QuakeAmmoType.h"
#include "QuakeCharacter.h"
#include "QuakeGameMode.h"
#include "QuakePlayerState.h"
#include "QuakeWeaponBase.h"

#include "Engine/World.h"
#include "Fonts/SlateFontInfo.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "QuakeHUDOverlay"

void SQuakeHUDOverlay::Construct(const FArguments& InArgs)
{
	OwningPlayerController = InArgs._OwningPlayerController;

	const FSlateFontInfo HealthFont = FCoreStyle::GetDefaultFontStyle("Bold", 36);
	const FSlateFontInfo WeaponFont = FCoreStyle::GetDefaultFontStyle("Regular", 18);
	const FSlateFontInfo AmmoFont   = FCoreStyle::GetDefaultFontStyle("Bold", 36);
	const FSlateFontInfo StatsFont  = FCoreStyle::GetDefaultFontStyle("Regular", 16);
	const FSlateFontInfo EndTitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 48);
	const FSlateFontInfo EndBodyFont  = FCoreStyle::GetDefaultFontStyle("Bold", 28);

	ChildSlot
	[
		SNew(SOverlay)

		// ---- Main in-game overlay ----
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)

			// Top-left stats strip (Phase 9).
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.f, 20.f, 0.f, 0.f)
			.HAlign(HAlign_Left)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Font(StatsFont)
					.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f))
					.Text(this, &SQuakeHUDOverlay::GetKillsText)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Font(StatsFont)
					.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f))
					.Text(this, &SQuakeHUDOverlay::GetSecretsText)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Font(StatsFont)
					.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f))
					.Text(this, &SQuakeHUDOverlay::GetTimeText)
				]
			]

			// Spacer pushes the bottom row to the bottom of the screen.
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SBox)
			]

			// Bottom row: HP + weapon name on the left, ammo on the right.
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(40.f, 0.f, 40.f, 40.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Left).VAlign(VAlign_Bottom)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Font(WeaponFont)
						.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f))
						.Text(this, &SQuakeHUDOverlay::GetWeaponText)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Font(HealthFont)
						.ColorAndOpacity(FLinearColor(1.f, 0.85f, 0.4f))
						.Text(this, &SQuakeHUDOverlay::GetHealthText)
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Right).VAlign(VAlign_Bottom)
				[
					SNew(STextBlock)
					.Font(AmmoFont)
					.ColorAndOpacity(FLinearColor(0.6f, 0.9f, 1.f))
					.Text(this, &SQuakeHUDOverlay::GetAmmoText)
				]
			]
		]

		// ---- Phase 9 level-end stats screen, shown on exit trigger ----
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.Visibility(this, &SQuakeHUDOverlay::GetLevelEndStatsVisibility)
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.75f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 24.f)
				[
					SNew(STextBlock)
					.Font(EndTitleFont)
					.ColorAndOpacity(FLinearColor(1.f, 0.85f, 0.4f))
					.Text(this, &SQuakeHUDOverlay::GetLevelEndTitleText)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(4.f)
				[
					SNew(STextBlock)
					.Font(EndBodyFont)
					.ColorAndOpacity(FLinearColor::White)
					.Text(this, &SQuakeHUDOverlay::GetLevelEndKillsText)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(4.f)
				[
					SNew(STextBlock)
					.Font(EndBodyFont)
					.ColorAndOpacity(FLinearColor::White)
					.Text(this, &SQuakeHUDOverlay::GetLevelEndSecretsText)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(4.f)
				[
					SNew(STextBlock)
					.Font(EndBodyFont)
					.ColorAndOpacity(FLinearColor::White)
					.Text(this, &SQuakeHUDOverlay::GetLevelEndTimeText)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(4.f)
				[
					SNew(STextBlock)
					.Font(EndBodyFont)
					.ColorAndOpacity(FLinearColor::White)
					.Text(this, &SQuakeHUDOverlay::GetLevelEndDeathsText)
				]
			]
		]
	];
}

void SQuakeHUDOverlay::ShowLevelEndStats(float Duration)
{
	const APlayerController* PC = OwningPlayerController.Get();
	if (!PC)
	{
		return;
	}
	if (const UWorld* World = PC->GetWorld())
	{
		LevelEndStatsExpireWorldTime = World->GetTimeSeconds() + Duration;
	}
}

const AQuakeCharacter* SQuakeHUDOverlay::ResolvePlayerCharacter() const
{
	if (const APlayerController* PC = OwningPlayerController.Get())
	{
		return Cast<AQuakeCharacter>(PC->GetPawn());
	}
	return nullptr;
}

AQuakePlayerState* SQuakeHUDOverlay::ResolvePlayerState() const
{
	if (APlayerController* PC = OwningPlayerController.Get())
	{
		return PC->GetPlayerState<AQuakePlayerState>();
	}
	return nullptr;
}

const AQuakeGameMode* SQuakeHUDOverlay::ResolveGameMode() const
{
	if (const APlayerController* PC = OwningPlayerController.Get())
	{
		if (const UWorld* World = PC->GetWorld())
		{
			return World->GetAuthGameMode<AQuakeGameMode>();
		}
	}
	return nullptr;
}

FText SQuakeHUDOverlay::GetHealthText() const
{
	if (const AQuakeCharacter* Char = ResolvePlayerCharacter())
	{
		return FText::FromString(FString::Printf(TEXT("HP %d"), FMath::RoundToInt(Char->GetHealth())));
	}
	return FText::FromString(TEXT("HP --"));
}

FText SQuakeHUDOverlay::GetWeaponText() const
{
	if (const AQuakeCharacter* Char = ResolvePlayerCharacter())
	{
		if (const AQuakeWeaponBase* Weapon = Char->CurrentWeapon)
		{
			return Weapon->DisplayName;
		}
	}
	return FText::GetEmpty();
}

FText SQuakeHUDOverlay::GetAmmoText() const
{
	const AQuakeCharacter* Char = ResolvePlayerCharacter();
	if (!Char)
	{
		return FText::GetEmpty();
	}
	const AQuakeWeaponBase* Weapon = Char->CurrentWeapon;
	if (!Weapon || Weapon->AmmoType == EQuakeAmmoType::None)
	{
		return FText::GetEmpty();
	}
	return FText::FromString(FString::Printf(TEXT("%d"), Char->GetAmmo(Weapon->AmmoType)));
}

FText SQuakeHUDOverlay::GetKillsText() const
{
	const AQuakePlayerState* PS = ResolvePlayerState();
	const AQuakeGameMode*    GM = ResolveGameMode();
	const int32 Num = PS ? PS->Kills : 0;
	const int32 Den = GM ? GM->KillsTotal : 0;
	return FText::FromString(FString::Printf(TEXT("Kills   %d / %d"), Num, Den));
}

FText SQuakeHUDOverlay::GetSecretsText() const
{
	const AQuakePlayerState* PS = ResolvePlayerState();
	const AQuakeGameMode*    GM = ResolveGameMode();
	const int32 Num = PS ? PS->Secrets : 0;
	const int32 Den = GM ? GM->SecretsTotal : 0;
	return FText::FromString(FString::Printf(TEXT("Secrets %d / %d"), Num, Den));
}

FText SQuakeHUDOverlay::GetTimeText() const
{
	const AQuakePlayerState* PS = ResolvePlayerState();
	const float Elapsed = PS ? PS->GetTimeElapsed() : 0.f;
	const int32 Total = FMath::Max(0, FMath::FloorToInt(Elapsed));
	return FText::FromString(FString::Printf(TEXT("Time    %02d:%02d"), Total / 60, Total % 60));
}

EVisibility SQuakeHUDOverlay::GetLevelEndStatsVisibility() const
{
	const APlayerController* PC = OwningPlayerController.Get();
	if (!PC)
	{
		return EVisibility::Collapsed;
	}
	const UWorld* World = PC->GetWorld();
	if (!World || World->GetTimeSeconds() >= LevelEndStatsExpireWorldTime)
	{
		return EVisibility::Collapsed;
	}
	return EVisibility::HitTestInvisible;
}

FText SQuakeHUDOverlay::GetLevelEndTitleText() const
{
	return LOCTEXT("LevelComplete", "Level Complete");
}

FText SQuakeHUDOverlay::GetLevelEndKillsText() const
{
	return GetKillsText();
}

FText SQuakeHUDOverlay::GetLevelEndSecretsText() const
{
	return GetSecretsText();
}

FText SQuakeHUDOverlay::GetLevelEndTimeText() const
{
	return GetTimeText();
}

FText SQuakeHUDOverlay::GetLevelEndDeathsText() const
{
	const AQuakePlayerState* PS = ResolvePlayerState();
	const int32 D = PS ? PS->Deaths : 0;
	return FText::FromString(FString::Printf(TEXT("Deaths  %d"), D));
}

#undef LOCTEXT_NAMESPACE
