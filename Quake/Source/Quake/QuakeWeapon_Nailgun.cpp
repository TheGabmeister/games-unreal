#include "QuakeWeapon_Nailgun.h"

#include "QuakeBalanceRows.h"
#include "QuakeCharacter.h"
#include "QuakeProjectile.h"
#include "QuakeSoundManager.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Perception/AISense_Hearing.h"

AQuakeWeapon_Nailgun::AQuakeWeapon_Nailgun()
{
	// SPEC section 2.0 Nailgun row:
	//   RoF:   8 shots/sec
	//   Ammo:  1 nail per shot
	RateOfFire = 8.f;
	AmmoType = EQuakeAmmoType::Nails;
	AmmoPerShot = 1;
	DisplayName = NSLOCTEXT("QuakeWeapon", "NailgunName", "Nailgun");
	StatsRowName = TEXT("Nailgun");
}

void AQuakeWeapon_Nailgun::ApplyStatsFromRow(const FQuakeWeaponStatsRow& Row)
{
	Super::ApplyStatsFromRow(Row);
	SpreadHalfAngleDegrees = Row.SpreadHalfAngleDegrees;
	MuzzleSpawnForwardOffset = Row.MuzzleSpawnForwardOffset;
	// Nailgun damage lives on AQuakeProjectile_Nail, not the weapon.
}

bool AQuakeWeapon_Nailgun::CanActuallyFire(AActor* /*InInstigator*/) const
{
	if (ProjectileClass != nullptr)
	{
		return true;
	}
	// Data-authoring bug — log from the pre-fire gate so TryFire's cooldown
	// arm rate-limits the spam (8 Hz here). See the matching pattern in
	// AQuakeWeapon_RocketLauncher::CanActuallyFire.
	UE_LOG(LogTemp, Warning,
		TEXT("AQuakeWeapon_Nailgun: ProjectileClass is null — assign BP_Projectile_Nail "
		     "in BP_Weapon_Nailgun defaults."));
	return false;
}

void AQuakeWeapon_Nailgun::Fire(AActor* InInstigator)
{
	// ProjectileClass is guaranteed non-null here: TryFire routes through
	// CanActuallyFire() before invoking Fire().
	APawn* PawnInstigator = nullptr;
	UWorld* World = nullptr;
	if (!GetFireContext(InInstigator, PawnInstigator, World))
	{
		return;
	}

	// Muzzle origin = firing pawn's eye viewpoint (same convention as every
	// other weapon — camera-aligned on the player).
	FVector EyeLoc;
	FRotator EyeRot;
	PawnInstigator->GetActorEyesViewPoint(EyeLoc, EyeRot);
	const FVector AimDir = EyeRot.Vector();

	// SPEC 2.0: 1° cone jitter. VRandCone returns a direction uniformly
	// distributed inside a cone of the given half-angle. Each trigger pull
	// gets one independent sample — the 8/sec RoF does the spread-feel
	// work across a held burst.
	const float SpreadRadians = FMath::DegreesToRadians(SpreadHalfAngleDegrees);
	const FVector NailDir = (SpreadRadians > 0.f)
		? FMath::VRandCone(AimDir, SpreadRadians)
		: AimDir;

	// SPEC 1.6 rule 1: 60 u muzzle spawn-out. Mirrors the Rocket Launcher.
	const FVector SpawnLocation = EyeLoc + AimDir * MuzzleSpawnForwardOffset;
	const FRotator SpawnRotation = NailDir.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = PawnInstigator;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AQuakeProjectile* Nail = World->SpawnActor<AQuakeProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams);

	// SPEC 4.3: bake Quad scale onto the shot at spawn time — a nail fired
	// during Quad lands at 4× even if the timer ticks out mid-flight.
	if (Nail)
	{
		if (const AQuakeCharacter* QuakePawn = Cast<AQuakeCharacter>(PawnInstigator))
		{
			Nail->DamageScale = QuakePawn->GetOutgoingDamageScale();
		}
	}

	// Hearing noise — softer than the shotgun's boom since the nailgun is a
	// sustained-fire weapon and reporting at 1.5 loudness 8 times a second
	// would dominate the perception system. 0.75 matches the "quiet stream
	// of pops" feel and still drags enemies into Chase from nearby rooms.
	UAISense_Hearing::ReportNoiseEvent(
		World,
		EyeLoc,
		/*Loudness*/ 0.75f,
		InInstigator,
		/*MaxRange*/ 0.f,
		FName(TEXT("QuakeWeaponFire")));

	UQuakeSoundManager::PlaySoundEvent(this, EQuakeSoundEvent::WeaponNailgunFire, EyeLoc);
}
