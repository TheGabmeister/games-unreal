#include "QuakeEnemy_Knight.h"

#include "QuakeAIController_Knight.h"
#include "QuakeCollisionChannels.h"
#include "QuakeDamageType_Melee.h"

#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

AQuakeEnemy_Knight::AQuakeEnemy_Knight()
{
	StatsRowName = TEXT("Knight");

	// SPEC section 3.1 Knight row — C++ fallback when no DataTable is loaded.
	MaxHealth = 75.f;
	WalkSpeed = 400.f;

	SightRadius = 2000.f;
	LoseSightRadius = 2200.f;
	HearingRadius = 1500.f;

	AttackRange = 80.f;
	AttackDamage = 10.f;
	AttackCooldown = 1.f;

	AIControllerClass = AQuakeAIController_Knight::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AQuakeEnemy_Knight::FireAtTarget(AActor* Target)
{
	if (!Target) return;
	UWorld* World = GetWorld();
	if (!World) return;

	// Short-range melee trace from the Knight's eye viewpoint toward the
	// target, same pattern as AQuakeWeapon_Axe::Fire.
	FVector EyeLoc;
	FRotator EyeRot;
	GetActorEyesViewPoint(EyeLoc, EyeRot);

	const FVector AimDir = (Target->GetActorLocation() - EyeLoc).GetSafeNormal();
	const FVector TraceEnd = EyeLoc + AimDir * AttackRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(QuakeKnightMelee), /*bTraceComplex*/ false, this);
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(
		Hit, EyeLoc, TraceEnd, QuakeCollision::ECC_Weapon, Params);

	if (!bHit || !Hit.GetActor()) return;

	UGameplayStatics::ApplyPointDamage(
		Hit.GetActor(),
		AttackDamage,
		AimDir,
		Hit,
		GetController(),
		/*DamageCauser*/ this,
		UQuakeDamageType_Melee::StaticClass());
}
