#include "QuakeWeapon_Nailgun.h"

#include "QuakeProjectile.h"

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
}

void AQuakeWeapon_Nailgun::Fire(AActor* InInstigator)
{
	APawn* PawnInstigator = Cast<APawn>(InInstigator);
	if (!PawnInstigator)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!ProjectileClass)
	{
		// A BP_Weapon_Nailgun that ships with no ProjectileClass set is a
		// data-authoring bug, not a runtime condition. Log loudly so the
		// editor's Output Log catches it on the first test fire, then bail
		// before spawning so we don't crash.
		UE_LOG(LogTemp, Warning,
			TEXT("AQuakeWeapon_Nailgun::Fire: ProjectileClass is null — assign BP_Projectile_Nail "
				 "in BP_Weapon_Nailgun defaults."));
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

	World->SpawnActor<AQuakeProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams);

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
}
