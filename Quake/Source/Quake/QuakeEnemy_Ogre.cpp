#include "QuakeEnemy_Ogre.h"

#include "QuakeAIController_Ogre.h"
#include "QuakeCollisionChannels.h"
#include "QuakeDamageType_Melee.h"
#include "QuakeProjectile.h"

#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

AQuakeEnemy_Ogre::AQuakeEnemy_Ogre()
{
	StatsRowName = TEXT("Ogre");

	// SPEC section 3.1 Ogre row — C++ fallback when no DataTable is loaded.
	MaxHealth = 200.f;
	WalkSpeed = 250.f;

	SightRadius = 2500.f;
	LoseSightRadius = 2700.f;
	HearingRadius = 2000.f;

	// AttackRange on the base is used by the FSM to decide when to enter
	// Attack state. Set to the GRENADE range (1500) so the Ogre starts
	// attacking from distance. The controller picks melee vs grenade based
	// on the actual distance inside the Attack state.
	AttackRange = 1500.f;
	AttackDamage = 40.f;   // grenade splash damage (SPEC 3.1)
	AttackCooldown = 2.f;

	AIControllerClass = AQuakeAIController_Ogre::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AQuakeEnemy_Ogre::FireAtTarget(AActor* Target)
{
	// Melee chainsaw — short-range trace, same pattern as Knight.
	if (!Target) return;
	UWorld* World = GetWorld();
	if (!World) return;

	FVector EyeLoc;
	FRotator EyeRot;
	GetActorEyesViewPoint(EyeLoc, EyeRot);

	const FVector AimDir = (Target->GetActorLocation() - EyeLoc).GetSafeNormal();
	const FVector TraceEnd = EyeLoc + AimDir * MeleeRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(QuakeOgreMelee), /*bTraceComplex*/ false, this);
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(
		Hit, EyeLoc, TraceEnd, QuakeCollision::ECC_Weapon, Params);

	if (!bHit || !Hit.GetActor()) return;

	UGameplayStatics::ApplyPointDamage(
		Hit.GetActor(),
		MeleeDamage,
		AimDir,
		Hit,
		GetController(),
		/*DamageCauser*/ this,
		UQuakeDamageType_Melee::StaticClass());
}

void AQuakeEnemy_Ogre::FireGrenadeAtTarget(AActor* Target)
{
	if (!Target || !GrenadeClass) return;
	UWorld* World = GetWorld();
	if (!World) return;

	FVector EyeLoc;
	FRotator EyeRot;
	GetActorEyesViewPoint(EyeLoc, EyeRot);

	// Aim at the target with a simple arc lob. The grenade's gravity
	// handles the arc — we just aim upward to compensate for drop.
	const FVector ToTarget = Target->GetActorLocation() - EyeLoc;
	const float Dist = ToTarget.Size2D();
	// Add upward pitch to lob the grenade in an arc. The 20° is a rough
	// approximation — good enough for Quake's enemy accuracy.
	FVector AimDir = ToTarget.GetSafeNormal();
	AimDir.Z = FMath::Max(AimDir.Z, 0.3f);  // minimum upward arc
	AimDir.Normalize();

	const FVector SpawnLoc = EyeLoc + AimDir * GrenadeSpawnForwardOffset;
	const FRotator SpawnRot = AimDir.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	World->SpawnActor<AQuakeProjectile>(GrenadeClass, SpawnLoc, SpawnRot, SpawnParams);
}
