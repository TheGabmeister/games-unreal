#include "QuakeWeapon_Shotgun.h"

#include "QuakeCollisionChannels.h"
#include "QuakeDamageType_Bullet.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"

AQuakeWeapon_Shotgun::AQuakeWeapon_Shotgun()
{
	// SPEC section 2.0 Shotgun row: 1.5 shots/sec, 1 shell per shot.
	RateOfFire = 1.5f;
	AmmoType = EQuakeAmmoType::Shells;
	AmmoPerShot = 1;
	DisplayName = NSLOCTEXT("QuakeWeapon", "ShotgunName", "Shotgun");
}

void AQuakeWeapon_Shotgun::Fire(AActor* InInstigator)
{
	APawn* PawnInstigator = nullptr;
	UWorld* World = nullptr;
	if (!GetFireContext(InInstigator, PawnInstigator, World))
	{
		return;
	}

	// Trace origin is the firing pawn's eye viewpoint (camera-aligned on
	// the player). Each pellet gets its own direction jittered inside a 4°
	// cone around the aim. Individual damage events per-pellet (not one
	// summed event) so knockback impulses and pain reactions stack
	// naturally at the target's TakeDamage.
	FVector EyeLoc;
	FRotator EyeRot;
	PawnInstigator->GetActorEyesViewPoint(EyeLoc, EyeRot);
	const FVector AimDir = EyeRot.Vector();

	FCollisionQueryParams Params(SCENE_QUERY_STAT(QuakeShotgunFire), /*bTraceComplex*/ false, this);
	Params.AddIgnoredActor(InInstigator);
	Params.AddIgnoredActor(this);

	AController* InstigatorController = PawnInstigator->GetController();

	const float SpreadRadians = FMath::DegreesToRadians(SpreadHalfAngleDegrees);

	for (int32 Pellet = 0; Pellet < PelletCount; ++Pellet)
	{
		// VRandCone produces a direction uniformly distributed inside a
		// cone of the given half-angle around the axis. It uses FMath::Rand
		// internally so each pellet gets an independent random sample.
		const FVector PelletDir = FMath::VRandCone(AimDir, SpreadRadians);
		const FVector TraceEnd = EyeLoc + PelletDir * Range;

		FHitResult Hit;
		const bool bHit = World->LineTraceSingleByChannel(
			Hit,
			EyeLoc,
			TraceEnd,
			QuakeCollision::ECC_Weapon,
			Params);

#if !UE_BUILD_SHIPPING
		// Dev visualization: orange pellet traces, short lifetime so the
		// spread cone is visible on a still frame but doesn't clutter.
		DrawDebugLine(
			World,
			EyeLoc,
			bHit ? Hit.ImpactPoint : TraceEnd,
			bHit ? FColor::Yellow : FColor(255, 140, 0),
			/*bPersistent*/ false,
			/*Lifetime*/ 0.25f,
			0,
			/*Thickness*/ 1.f);
#endif

		if (bHit && Hit.GetActor())
		{
			UGameplayStatics::ApplyPointDamage(
				Hit.GetActor(),
				DamagePerPellet,
				PelletDir,
				Hit,
				InstigatorController,
				this,
				UQuakeDamageType_Bullet::StaticClass());
		}
	}

	// One noise event per shot (not per pellet) — six noise reports on a
	// single pull of the trigger would spam the perception system and
	// cause duplicated stimulus updates on nearby enemies.
	UAISense_Hearing::ReportNoiseEvent(
		World,
		EyeLoc,
		/*Loudness*/ 1.5f,   // Louder than the Axe's 1.0 to reflect the boom
		InInstigator,
		/*MaxRange*/ 0.f,
		FName(TEXT("QuakeWeaponFire")));
}
