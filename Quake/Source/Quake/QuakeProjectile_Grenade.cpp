#include "QuakeProjectile_Grenade.h"

#include "QuakeDamageType_Explosive.h"

#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AQuakeProjectile_Grenade::AQuakeProjectile_Grenade()
{
	// SPEC 2.0 Grenade Launcher row: 800 u/s, arcing (gravity 1.0).
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = 800.f;
		ProjectileMovement->MaxSpeed     = 800.f;
		ProjectileMovement->ProjectileGravityScale = 1.f;
		ProjectileMovement->bShouldBounce = true;
		ProjectileMovement->Bounciness   = 0.4f;
		// Friction slows the grenade on each bounce so it settles rather
		// than sliding forever. Matches the feel of original Quake grenades.
		ProjectileMovement->Friction     = 0.5f;
	}

	if (CollisionSphere)
	{
		CollisionSphere->InitSphereRadius(6.f);
	}

	// Bounce delegate for sound stub.
	if (ProjectileMovement)
	{
		ProjectileMovement->OnProjectileBounce.AddDynamic(
			this, &AQuakeProjectile_Grenade::OnGrenadeBounce);
	}
}

void AQuakeProjectile_Grenade::BeginPlay()
{
	Super::BeginPlay();

	// Start the fuse timer — explodes after FuseTime regardless of bounces.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FuseTimerHandle, this, &AQuakeProjectile_Grenade::OnFuseExpired,
			FuseTime, /*bLoop*/ false);

		FirerGraceEndTime = World->GetTimeSeconds() + FirerGracePeriod;
	}

	// Unlike rockets, grenades do NOT permanently ignore the firer.
	// The base class BeginPlay sets IgnoreActorWhenMoving(Firer, true)
	// to prevent spawn-frame self-detonation. We'll re-enable firer
	// collision after the grace period in HandleImpact checks.
}

void AQuakeProjectile_Grenade::HandleImpact(const FHitResult& Hit, AActor* OtherActor)
{
	// World geometry hit (no actor, or non-Pawn actor like a BSP brush):
	// let the bounce happen — do NOT explode. The bounce delegate
	// OnGrenadeBounce handles the sound stub.
	if (!OtherActor)
	{
		return;
	}

	// Only explode on Pawn-channel actors (characters — player or enemies).
	// Other actors (static meshes, triggers, pickups) are ignored.
	if (!OtherActor->IsA(ACharacter::StaticClass()))
	{
		return;
	}

	// Firer grace period: ignore the firer for the first 0.25 s so the
	// grenade clears the capsule. After that, walking into your own
	// grenade detonates it (matching original Quake).
	if (OtherActor == GetInstigator())
	{
		const UWorld* World = GetWorld();
		if (World && World->GetTimeSeconds() < FirerGraceEndTime)
		{
			return;
		}
	}

	Explode();
}

void AQuakeProjectile_Grenade::OnFuseExpired()
{
	Explode();
}

void AQuakeProjectile_Grenade::Explode()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		Destroy();
		return;
	}

	// Cancel the fuse timer if exploding from contact (prevent double-explode).
	World->GetTimerManager().ClearTimer(FuseTimerHandle);

	const FVector ExplosionOrigin = GetActorLocation();

	APawn* FiringPawn = GetInstigator();
	AController* InstigatorController = FiringPawn ? FiringPawn->GetController() : nullptr;

	// Same radial damage parameters as the Rocket (SPEC 2.2: 120 u splash,
	// linear falloff, UQuakeDamageType_Explosive).
	const TArray<AActor*> IgnoreActors;
	UGameplayStatics::ApplyRadialDamageWithFalloff(
		this,
		BaseDamage * DamageScale,
		/*MinimumDamage*/     0.f,
		ExplosionOrigin,
		/*DamageInnerRadius*/ 0.f,
		/*DamageOuterRadius*/ SplashRadius,
		/*DamageFalloff*/     1.f,
		UQuakeDamageType_Explosive::StaticClass(),
		IgnoreActors,
		/*DamageCauser*/ this,
		InstigatorController);

	Destroy();
}

void AQuakeProjectile_Grenade::OnGrenadeBounce(const FHitResult& /*ImpactResult*/, const FVector& /*ImpactVelocity*/)
{
	// Phase 7 stub — real bounce sound will be wired via the audio system
	// (Phase 14). Log at Verbose so PIE debugging can confirm bounces fire.
	UE_LOG(LogTemp, Verbose, TEXT("%s: bounce"), *GetName());

	// After the grace period, re-enable firer collision so walking into
	// the grenade post-bounce detonates it.
	if (AActor* Firer = GetInstigator())
	{
		if (const UWorld* World = GetWorld())
		{
			if (World->GetTimeSeconds() >= FirerGraceEndTime && CollisionSphere)
			{
				CollisionSphere->IgnoreActorWhenMoving(Firer, /*bShouldIgnore*/ false);
			}
		}
	}
}
