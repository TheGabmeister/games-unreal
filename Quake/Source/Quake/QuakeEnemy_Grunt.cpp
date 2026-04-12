#include "QuakeEnemy_Grunt.h"

#include "QuakeAIController_Grunt.h"
#include "QuakeCollisionChannels.h"
#include "QuakeDamageType_Bullet.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

AQuakeEnemy_Grunt::AQuakeEnemy_Grunt()
{
	StatsRowName = TEXT("Grunt");

	// SPEC section 3.1 Grunt row — C++ fallback when no DataTable is loaded.
	MaxHealth = 30.f;
	WalkSpeed = 300.f;

	SightRadius = 2000.f;
	LoseSightRadius = 2200.f;
	HearingRadius = 1500.f;

	AttackRange = 1500.f;
	AttackDamage = 4.f;
	AttackCooldown = 1.5f;

	// Wire our brain. The SpawnPoint / placement in the editor will possess
	// us automatically once AutoPossessAI fires.
	AIControllerClass = AQuakeAIController_Grunt::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AQuakeEnemy_Grunt::FireAtTarget(AActor* Target)
{
	if (!Target)
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Muzzle is approximated as (actor location + eye-level Z offset). A
	// proper muzzle socket comes when real meshes replace the primitive stubs;
	// for now this is enough to produce a believable trace from roughly
	// where the rifle box sits.
	const FVector MuzzleLoc = GetActorLocation() + FVector(0.f, 0.f, 50.f);
	const FVector AimDir = (Target->GetActorLocation() - MuzzleLoc).GetSafeNormal();
	const FVector TraceEnd = MuzzleLoc + AimDir * AttackRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(QuakeGruntFire), /*bTraceComplex*/ false, this);
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(
		Hit,
		MuzzleLoc,
		TraceEnd,
		QuakeCollision::ECC_Weapon,
		Params);

#if !UE_BUILD_SHIPPING
	// Dev visualization: yellow on hit, orange on miss. Short lifetime so
	// repeated fire doesn't clutter PIE. Stripped from shipping builds.
	DrawDebugLine(
		World,
		MuzzleLoc,
		bHit ? Hit.ImpactPoint : TraceEnd,
		bHit ? FColor::Yellow : FColor::Orange,
		/*bPersistent*/ false,
		/*Lifetime*/ 0.3f,
		0,
		/*Thickness*/ 1.f);
#endif

	if (!bHit || !Hit.GetActor())
	{
		return;
	}

	// SPEC section 1.5 damage attribution: EventInstigator = the firing
	// pawn's controller, DamageCauser = this pawn (not a separate weapon
	// actor — the Grunt's rifle is a visual primitive, not an AActor).
	UGameplayStatics::ApplyPointDamage(
		Hit.GetActor(),
		AttackDamage,
		AimDir,
		Hit,
		GetController(),
		/*DamageCauser*/ this,
		UQuakeDamageType_Bullet::StaticClass());
}
