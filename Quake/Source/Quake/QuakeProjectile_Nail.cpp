#include "QuakeProjectile_Nail.h"

#include "QuakeDamageType_Nail.h"

#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AQuakeProjectile_Nail::AQuakeProjectile_Nail()
{
	// SPEC section 2.0 Nailgun row:
	//   Speed: 1500 u/s, straight (no gravity)
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = 1500.f;
		ProjectileMovement->MaxSpeed     = 1500.f;
		ProjectileMovement->ProjectileGravityScale = 0.f;
	}

	// Nails are thin — smaller collision sphere than the rocket so they
	// weave past walls naturally. 2 u keeps the feel precise while still
	// being large enough that a head-on hit at 1500 u/s never tunnels
	// through the target's capsule on a single frame.
	if (CollisionSphere)
	{
		CollisionSphere->InitSphereRadius(2.f);
	}
}

void AQuakeProjectile_Nail::HandleImpact(const FHitResult& Hit, AActor* OtherActor)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		Destroy();
		return;
	}

	// Direct damage on whatever we hit (nails don't splash). World geometry
	// hits skip the damage call entirely — the nail still destroys itself,
	// but there's no actor to damage.
	if (OtherActor)
	{
		APawn* FiringPawn = GetInstigator();
		AController* InstigatorController = FiringPawn ? FiringPawn->GetController() : nullptr;

		// Shot direction from the velocity — this is what the base's
		// bRotationFollowsVelocity keeps the actor's forward aligned to, so
		// it's also the best knockback axis for TakeDamage's GetBestHitInfo.
		const FVector ShotDirection = GetVelocity().GetSafeNormal();

		UGameplayStatics::ApplyPointDamage(
			OtherActor,
			BaseDamage * DamageScale,
			ShotDirection,
			Hit,
			InstigatorController,
			this,
			UQuakeDamageType_Nail::StaticClass());
	}

	// One-shot: the nail is gone on any impact (world or actor).
	Destroy();
}
