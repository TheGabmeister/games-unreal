#include "SQuakeHUDOverlay.h"

#include "QuakeAmmoType.h"
#include "QuakeCharacter.h"
#include "QuakeWeaponBase.h"

#include "Fonts/SlateFontInfo.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "QuakeHUDOverlay"

void SQuakeHUDOverlay::Construct(const FArguments& InArgs)
{
	PlayerCharacter = InArgs._PlayerCharacter;

	const FSlateFontInfo HealthFont = FCoreStyle::GetDefaultFontStyle("Bold", 36);
	const FSlateFontInfo WeaponFont = FCoreStyle::GetDefaultFontStyle("Regular", 18);
	const FSlateFontInfo AmmoFont   = FCoreStyle::GetDefaultFontStyle("Bold", 36);

	// Bottom-left: HP number + weapon-name subtitle. Bottom-right: ammo
	// number for the current weapon. SPEC section 7 HUD layout positions
	// will be refined in Phase 10; this is the functional minimum so Phase
	// 4 verification ("see HUD ammo count change") can happen.
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)  // Spacer pushes the readout row to the bottom.
		[
			SNew(SBox)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(40.f, 0.f, 40.f, 40.f)
		[
			SNew(SHorizontalBox)

			// Left cluster: HP + weapon name.
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Font(WeaponFont)
					.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f))
					.Text(this, &SQuakeHUDOverlay::GetWeaponText)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Font(HealthFont)
					.ColorAndOpacity(FLinearColor(1.f, 0.85f, 0.4f))
					.Text(this, &SQuakeHUDOverlay::GetHealthText)
				]
			]

			// Right cluster: ammo count (blank for Axe).
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			[
				SNew(STextBlock)
				.Font(AmmoFont)
				.ColorAndOpacity(FLinearColor(0.6f, 0.9f, 1.f))
				.Text(this, &SQuakeHUDOverlay::GetAmmoText)
			]
		]
	];
}

FText SQuakeHUDOverlay::GetHealthText() const
{
	if (const AQuakeCharacter* Char = PlayerCharacter.Get())
	{
		return FText::FromString(FString::Printf(TEXT("HP %d"), FMath::RoundToInt(Char->GetHealth())));
	}
	return FText::FromString(TEXT("HP --"));
}

FText SQuakeHUDOverlay::GetWeaponText() const
{
	if (const AQuakeCharacter* Char = PlayerCharacter.Get())
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
	const AQuakeCharacter* Char = PlayerCharacter.Get();
	if (!Char)
	{
		return FText::GetEmpty();
	}
	const AQuakeWeaponBase* Weapon = Char->CurrentWeapon;
	if (!Weapon || Weapon->AmmoType == EQuakeAmmoType::None)
	{
		// Axe: no ammo readout. Blank rather than "--" so it doesn't
		// visually compete with the HP number.
		return FText::GetEmpty();
	}
	return FText::FromString(FString::Printf(TEXT("%d"), Char->GetAmmo(Weapon->AmmoType)));
}

#undef LOCTEXT_NAMESPACE
