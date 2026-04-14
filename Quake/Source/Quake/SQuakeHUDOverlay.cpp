#include "SQuakeHUDOverlay.h"

#include "QuakeAmmoType.h"
#include "QuakeCharacter.h"
#include "QuakeGameInstance.h"
#include "QuakeGameMode.h"
#include "QuakePlayerState.h"
#include "QuakePowerup.h"
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

			// Top-right stats strip (Phase 9). Right-aligned so it doesn't
			// collide with the Phase 1 debug speedometer DrawHUD paints at
			// top-left. Move back to HAlign_Left once the speedometer is
			// removed (see CLAUDE.md Risk Note: Strafe-Jumping CMC).
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 20.f, 20.f, 0.f)
			.HAlign(HAlign_Right)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.Font(StatsFont)
					.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f))
					.Text(this, &SQuakeHUDOverlay::GetKillsText)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.Font(StatsFont)
					.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f))
					.Text(this, &SQuakeHUDOverlay::GetSecretsText)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
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

			// Top-center Quad timer (Phase 10). SPEC 7: "Powerup Timer —
			// top-center — active powerup icon + countdown". Collapsed when
			// no Quad is active.
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 20.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Font(EndBodyFont)
				.ColorAndOpacity(FLinearColor(0.4f, 0.6f, 1.f))  // Blue for Quad.
				.Visibility(this, &SQuakeHUDOverlay::GetQuadTimerVisibility)
				.Text(this, &SQuakeHUDOverlay::GetQuadTimerText)
			]

			// Bottom row: HP/armor/keys on the left, ammo on the right.
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
						// Silver / Gold key indicators on a single line.
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
						[
							SNew(STextBlock)
							.Font(WeaponFont)
							.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.85f))
							.Visibility(this, &SQuakeHUDOverlay::GetSilverKeyVisibility)
							.Text(this, &SQuakeHUDOverlay::GetSilverKeyText)
						]
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(STextBlock)
							.Font(WeaponFont)
							.ColorAndOpacity(FLinearColor(1.f, 0.84f, 0.f))
							.Visibility(this, &SQuakeHUDOverlay::GetGoldKeyVisibility)
							.Text(this, &SQuakeHUDOverlay::GetGoldKeyText)
						]
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Font(WeaponFont)
						.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f))
						.Text(this, &SQuakeHUDOverlay::GetWeaponText)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(STextBlock)
							.Font(HealthFont)
							.ColorAndOpacity(FLinearColor(1.f, 0.85f, 0.4f))
							.Text(this, &SQuakeHUDOverlay::GetHealthText)
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(30.f, 0.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Font(HealthFont)
							.ColorAndOpacity(this, &SQuakeHUDOverlay::GetArmorColor)
							.Visibility(this, &SQuakeHUDOverlay::GetArmorVisibility)
							.Text(this, &SQuakeHUDOverlay::GetArmorText)
						]
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

namespace
{
	// SPEC 4.2 tier thresholds: absorption ratio → HUD color.
	FLinearColor ArmorColorFromAbsorption(float Absorption)
	{
		if (Absorption >= 0.75f) { return FLinearColor(1.f, 0.25f, 0.25f); }  // Red
		if (Absorption >= 0.55f) { return FLinearColor(1.f, 0.9f,  0.2f);  }  // Yellow
		return FLinearColor(0.3f, 1.f, 0.3f);                                 // Green
	}
}

FText SQuakeHUDOverlay::GetArmorText() const
{
	const AQuakeCharacter* Char = ResolvePlayerCharacter();
	const UWorld* World = Char ? Char->GetWorld() : nullptr;
	const UQuakeGameInstance* GI = World ? World->GetGameInstance<UQuakeGameInstance>() : nullptr;
	if (!GI)
	{
		return FText::GetEmpty();
	}
	return FText::FromString(FString::Printf(TEXT("AR %d"), FMath::RoundToInt(GI->Armor)));
}

FSlateColor SQuakeHUDOverlay::GetArmorColor() const
{
	const AQuakeCharacter* Char = ResolvePlayerCharacter();
	const UWorld* World = Char ? Char->GetWorld() : nullptr;
	const UQuakeGameInstance* GI = World ? World->GetGameInstance<UQuakeGameInstance>() : nullptr;
	return FSlateColor(ArmorColorFromAbsorption(GI ? GI->ArmorAbsorption : 0.f));
}

EVisibility SQuakeHUDOverlay::GetArmorVisibility() const
{
	const AQuakeCharacter* Char = ResolvePlayerCharacter();
	const UWorld* World = Char ? Char->GetWorld() : nullptr;
	const UQuakeGameInstance* GI = World ? World->GetGameInstance<UQuakeGameInstance>() : nullptr;
	return (GI && GI->Armor > 0.f) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SQuakeHUDOverlay::GetSilverKeyText() const
{
	return NSLOCTEXT("QuakeHUD", "KeySilver", "[SILVER]");
}

EVisibility SQuakeHUDOverlay::GetSilverKeyVisibility() const
{
	const AQuakePlayerState* PS = ResolvePlayerState();
	return (PS && PS->HasKey(EQuakeKeyColor::Silver)) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SQuakeHUDOverlay::GetGoldKeyText() const
{
	return NSLOCTEXT("QuakeHUD", "KeyGold", "[GOLD]");
}

EVisibility SQuakeHUDOverlay::GetGoldKeyVisibility() const
{
	const AQuakePlayerState* PS = ResolvePlayerState();
	return (PS && PS->HasKey(EQuakeKeyColor::Gold)) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SQuakeHUDOverlay::GetQuadTimerText() const
{
	const AQuakePlayerState* PS = ResolvePlayerState();
	const float Remaining = PS ? PS->GetPowerupRemaining(EQuakePowerup::Quad) : 0.f;
	return FText::FromString(FString::Printf(TEXT("QUAD  %d"), FMath::CeilToInt(Remaining)));
}

EVisibility SQuakeHUDOverlay::GetQuadTimerVisibility() const
{
	const AQuakePlayerState* PS = ResolvePlayerState();
	return (PS && PS->HasPowerup(EQuakePowerup::Quad)) ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE
