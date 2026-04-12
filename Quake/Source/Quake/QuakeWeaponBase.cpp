#include "QuakeWeaponBase.h"

#include "QuakeCharacter.h"
#include "QuakeGameInstance.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Perception/AISense_Hearing.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeWeapon, Log, All);

AQuakeWeaponBase::AQuakeWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ViewModelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ViewModelMesh"));
	RootComponent = ViewModelMesh;

	// The viewmodel never collides — it's a visual attachment to the camera.
	ViewModelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ViewModelMesh->SetGenerateOverlapEvents(false);
	ViewModelMesh->SetCastShadow(false);
}

bool AQuakeWeaponBase::CanFire() const
{
	if (LastFireWorldTime < 0.0)
	{
		return true;  // Never fired before — first shot is always free.
	}
	const double Cooldown = 1.0 / FMath::Max(RateOfFire, KINDA_SMALL_NUMBER);
	return (GetWorld()->GetTimeSeconds() - LastFireWorldTime) >= Cooldown;
}

bool AQuakeWeaponBase::TryFire(AActor* InInstigator)
{
	if (!CanFire())
	{
		return false;
	}

	// Ammo gate. Weapons with AmmoType::None (the Axe) bypass this entirely
	// — ConsumeAmmo(None, ...) returns true without touching the TMap. For
	// ammo weapons, a failed consume plays the empty click, re-arms the
	// cooldown so holding fire on empty spams click at the weapon's RoF
	// (matching Quake) rather than at the input tick rate, and asks the
	// firing character to auto-switch to the next best owned weapon per
	// SPEC 2.2.
	if (AmmoType != EQuakeAmmoType::None && AmmoPerShot > 0)
	{
		UQuakeGameInstance* GameInstance =
			GetWorld() ? GetWorld()->GetGameInstance<UQuakeGameInstance>() : nullptr;
		const bool bHasAmmo = GameInstance && GameInstance->ConsumeAmmo(AmmoType, AmmoPerShot);
		if (!bHasAmmo)
		{
			PlayEmptyClick(InInstigator);
			LastFireWorldTime = GetWorld()->GetTimeSeconds();

			// SPEC 2.2 auto-switch on empty: walk the priority list
			// (RL → SNG → SSG → NG → SG → Axe) and swap to the first
			// owned weapon with fireable ammo. The click still plays
			// before the swap so the player gets the "out of ammo" cue.
			if (AQuakeCharacter* Character = Cast<AQuakeCharacter>(InInstigator))
			{
				Character->AutoSwitchFromEmptyWeapon();
			}
			return false;
		}
	}

	LastFireWorldTime = GetWorld()->GetTimeSeconds();
	Fire(InInstigator);
	return true;
}

void AQuakeWeaponBase::PlayEmptyClick(AActor* InInstigator)
{
	UE_LOG(LogQuakeWeapon, Verbose, TEXT("%s: *click* (empty)"), *GetName());

	// Failed fire still makes a (quiet) noise event. SPEC 3.3 wants enemies
	// to hear the player making audible mistakes; this is the dry-fire
	// equivalent of the weapon-fire noise in QuakeWeapon_Axe.cpp. A future
	// polish pass can wire an actual click sound asset via the audio
	// stub system (Phase 14).
	if (UWorld* World = GetWorld())
	{
		const FVector Origin = InInstigator ? InInstigator->GetActorLocation() : GetActorLocation();
		UAISense_Hearing::ReportNoiseEvent(
			World,
			Origin,
			/*Loudness*/ 0.25f,
			InInstigator,
			/*MaxRange*/ 0.f,
			FName(TEXT("QuakeWeaponClick")));
	}
}
