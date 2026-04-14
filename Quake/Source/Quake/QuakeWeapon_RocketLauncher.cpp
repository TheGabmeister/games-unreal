#include "QuakeWeapon_RocketLauncher.h"

#include "QuakeBalanceRows.h"
#include "QuakeCharacter.h"
#include "QuakeProjectile.h"
#include "QuakeSoundManager.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"

AQuakeWeapon_RocketLauncher::AQuakeWeapon_RocketLauncher()
{
	// SPEC section 2.0 Rocket Launcher row:
	//   RoF:   1.5 shots/sec
	//   Ammo:  1 rocket per shot
	RateOfFire = 1.5f;
	AmmoType = EQuakeAmmoType::Rockets;
	AmmoPerShot = 1;
	DisplayName = NSLOCTEXT("QuakeWeapon", "RocketLauncherName", "Rocket Launcher");
	StatsRowName = TEXT("RocketLauncher");
}

void AQuakeWeapon_RocketLauncher::ApplyStatsFromRow(const FQuakeWeaponStatsRow& Row)
{
	Super::ApplyStatsFromRow(Row);
	MuzzleSpawnForwardOffset = Row.MuzzleSpawnForwardOffset;
	// SplashRadius and BaseDamage live on AQuakeProjectile_Rocket.
	// Row.SplashRadius is informational for designers; the weapon does not
	// own that stat.
}

bool AQuakeWeapon_RocketLauncher::CanActuallyFire(AActor* /*InInstigator*/) const
{
	if (ProjectileClass != nullptr)
	{
		return true;
	}
	// A BP_Weapon_RocketLauncher shipped with no ProjectileClass set is a
	// data-authoring bug. Log from here — TryFire's cooldown arm gates the
	// spam to the weapon's RoF (1.5 Hz), so the authoring bug is visible
	// in the Output Log without drowning the editor.
	UE_LOG(LogTemp, Warning,
		TEXT("AQuakeWeapon_RocketLauncher: ProjectileClass is null — assign BP_Projectile_Rocket "
		     "in BP_Weapon_RocketLauncher defaults."));
	return false;
}

void AQuakeWeapon_RocketLauncher::Fire(AActor* InInstigator)
{
	// ProjectileClass is guaranteed non-null here: TryFire routes through
	// CanActuallyFire() before invoking Fire().
	APawn* PawnInstigator = nullptr;
	UWorld* World = nullptr;
	if (!GetFireContext(InInstigator, PawnInstigator, World))
	{
		return;
	}

	// Muzzle origin is the firing pawn's eye viewpoint (same convention as
	// the Shotgun and Axe — this is camera-aligned on the player).
	FVector EyeLoc;
	FRotator EyeRot;
	PawnInstigator->GetActorEyesViewPoint(EyeLoc, EyeRot);
	const FVector AimDir = EyeRot.Vector();

	// SPEC 1.6 rule 1: project 60 u in front of the pawn BEFORE spawning so
	// the first sweep tick doesn't start inside the firer's capsule. The
	// rocket's BeginPlay also calls IgnoreActorWhenMoving(Firer) as a second
	// guard, but the 60 u offset is the primary defense.
	const FVector SpawnLocation = EyeLoc + AimDir * MuzzleSpawnForwardOffset;
	const FRotator SpawnRotation = AimDir.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = PawnInstigator;
	// AlwaysSpawn: never refuse due to encroachment. If a rocket spawns
	// clipping a wall (rare — the 60 u offset usually prevents this) the
	// sphere's first sweep tick will detonate it against the wall on the
	// next frame, which is the correct Quake behavior.
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AQuakeProjectile* Rocket = World->SpawnActor<AQuakeProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams);

	// SPEC 4.3: bake Quad scale onto the rocket at launch — self-damage scale
	// is applied separately in TakeDamage via UQuakeDamageType_Explosive's
	// SelfDamageScale = 0.5, so rocket-jump damage is Quad × 0.5, not Quad.
	if (Rocket)
	{
		if (const AQuakeCharacter* QuakePawn = Cast<AQuakeCharacter>(PawnInstigator))
		{
			Rocket->DamageScale = QuakePawn->GetOutgoingDamageScale();
		}
	}

	// Hearing noise event — same pattern as the Shotgun / Axe. Loudness 2.0
	// since a rocket launch is substantially louder than a shotgun blast
	// (which is 1.5); this keeps enemies aggro'ing to rocket fire from
	// farther away. Range 0 defers to the AI sense config's HearingRange.
	UAISense_Hearing::ReportNoiseEvent(
		World,
		EyeLoc,
		/*Loudness*/ 2.0f,
		InInstigator,
		/*MaxRange*/ 0.f,
		FName(TEXT("QuakeWeaponFire")));

	UQuakeSoundManager::PlaySoundEvent(this, EQuakeSoundEvent::WeaponRocketFire, EyeLoc);
}
