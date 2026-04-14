#include "QuakeProjectile_Rocket.h"

#include "QuakeDamageType_Explosive.h"
#include "QuakeSoundManager.h"

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

	// DamageInnerRadius is non-zero on purpose: UE's ApplyRadialDamageWithFalloff
	// measures distance from Origin to the victim's component-location (capsule
	// center), not to the nearest surface point — so a direct hit on a Grunt's
	// 35 u / 90 u capsule reports ~37 u of distance under the hood and only
	// ~69 damage with InnerRadius=0. A 60 u plateau swallows that measurement
	// quirk so direct hits land at full BaseDamage without needing a separate
	// ApplyPointDamage call. Falloff stays linear from InnerRadius to SplashRadius.
	// See CLAUDE.md "Radial damage measures to component center, not surface"
	// for the full explanation.
	const TArray<AActor*> IgnoreActors;
	UGameplayStatics::ApplyRadialDamageWithFalloff(
		this,
		BaseDamage * DamageScale,
		/*MinimumDamage*/ 0.f,
		ExplosionOrigin,
		DamageInnerRadius,
		/*DamageOuterRadius*/ SplashRadius,
		/*DamageFalloff*/     1.f,
		UQuakeDamageType_Explosive::StaticClass(),
		IgnoreActors,
		/*DamageCauser*/ this,
		InstigatorController);

	UQuakeSoundManager::PlaySoundEvent(this, EQuakeSoundEvent::RocketExplode, ExplosionOrigin);

	Destroy();
}
