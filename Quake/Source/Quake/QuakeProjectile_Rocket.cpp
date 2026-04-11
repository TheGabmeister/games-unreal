#include "QuakeProjectile_Rocket.h"

#include "QuakeDamageType_Explosive.h"

#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AQuakeProjectile_Rocket::AQuakeProjectile_Rocket()
{
	// SPEC section 2.0 Rocket Launcher row:
	//   Speed: 1000 u/s, straight (no gravity)
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = 1000.f;
		ProjectileMovement->MaxSpeed     = 1000.f;
		ProjectileMovement->ProjectileGravityScale = 0.f;
	}

	// Slightly larger collision sphere than the base default so a glancing
	// shot at a target's feet still registers — 8 u matches the "chunky
	// rocket" feel of original Quake without being a bullet magnet.
	if (CollisionSphere)
	{
		CollisionSphere->InitSphereRadius(8.f);
	}
}

float AQuakeProjectile_Rocket::ComputeLinearFalloffDamage(float InBaseDamage, float InDistance, float InRadius)
{
	// Guard against degenerate radius (a zero-radius rocket would divide by
	// zero below). Treat it as "no splash at all".
	if (InRadius <= 0.f)
	{
		return 0.f;
	}

	// Outside the radius: no damage.
	if (InDistance >= InRadius)
	{
		return 0.f;
	}

	// At or behind the center: full damage. The clamp below handles this
	// uniformly too, but the short-circuit documents intent.
	if (InDistance <= 0.f)
	{
		return InBaseDamage;
	}

	// Linear falloff: full at center, 0 at the edge. Matches
	// ApplyRadialDamageWithFalloff with InnerRadius=0, DamageFalloff=1.0.
	const float Scale = 1.f - (InDistance / InRadius);
	return InBaseDamage * Scale;
}

void AQuakeProjectile_Rocket::HandleImpact(const FHitResult& Hit, AActor* /*OtherActor*/)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		Destroy();
		return;
	}

	// Explode at the hit location — not at the rocket's actor location,
	// because on a grazing hit those can differ by the sphere radius.
	const FVector ExplosionOrigin = Hit.ImpactPoint;

	// Attribution: the instigator controller is the firer, the damage causer
	// is the rocket itself. AQuakeCharacter::TakeDamage reads EventInstigator
	// for self-damage detection and EventInstigator->GetPawn() for the
	// self-damage scale; routing both through here keeps the rocket-jump
	// path working (player fires, player takes scaled splash, launched up).
	APawn* FiringPawn = GetInstigator();
	AController* InstigatorController = FiringPawn ? FiringPawn->GetController() : nullptr;

	// Don't double-count the direct-hit victim: ApplyRadialDamageWithFalloff
	// at InnerRadius=0 already gives them full BaseDamage because their
	// distance to ExplosionOrigin is effectively 0. No separate
	// ApplyPointDamage call is needed (and adding one would double the
	// direct-hit damage, which is wrong).
	//
	// DamageFalloff = 1.0 is a LINEAR falloff (damage = BaseDamage * (1 - d/r)
	// at InnerRadius 0). ComputeLinearFalloffDamage above mirrors this and
	// is the function the unit tests exercise.
	const TArray<AActor*> IgnoreActors;
	UGameplayStatics::ApplyRadialDamageWithFalloff(
		this,
		BaseDamage,
		/*MinimumDamage*/ 0.f,
		ExplosionOrigin,
		/*DamageInnerRadius*/ 0.f,
		/*DamageOuterRadius*/ SplashRadius,
		/*DamageFalloff*/     1.f,
		UQuakeDamageType_Explosive::StaticClass(),
		IgnoreActors,
		/*DamageCauser*/ this,
		InstigatorController);

	// One-shot: the rocket is gone. Later phases add an explosion VFX /
	// sound via a spawned cosmetic actor before Destroy; for Phase 5 the
	// rocket is a bare primitive so just end the actor here.
	Destroy();
}
