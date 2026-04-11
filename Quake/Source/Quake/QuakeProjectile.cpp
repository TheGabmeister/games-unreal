#include "QuakeProjectile.h"

#include "QuakeCollisionChannels.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AQuakeProjectile::AQuakeProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Sphere is the root + collision volume. Small default radius (4 u) so a
	// subclass that forgets to retune still has a sensible skinny projectile;
	// Rocket overrides this in its ctor.
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(4.f);
	RootComponent = CollisionSphere;

	// SPEC 1.6 object response matrix, "Projectile sphere" row:
	//   Object type = Projectile
	//   Block  WorldStatic, WorldDynamic, Pawn, Corpse
	//   Ignore Pickup, Projectile (so two rockets don't detonate each other),
	//          Visibility, Weapon (the hitscan trace channel)
	// Rationale for Ignore Corpse row-wise: corpses already ignore Projectile
	// on THEIR side (rule 2), but we still set this side to keep the matrix
	// symmetric as documented.
	// We start from Ignore-all and flip only what we block, so a channel
	// added later (e.g. future Gibs channel) defaults to Ignore.
	CollisionSphere->SetCollisionObjectType(QuakeCollision::ECC_Projectile);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic,  ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn,         ECR_Block);
	// Corpses opt out on their own side too (SPEC 1.6 rule 2 sets Corpse row
	// to Ignore Projectile). Keep this explicit so the hit still delivers if
	// a corpse's response is ever changed back.
	CollisionSphere->SetCollisionResponseToChannel(QuakeCollision::ECC_Corpse, ECR_Block);
	CollisionSphere->SetNotifyRigidBodyCollision(true);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionSphere);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetCastShadow(false);

	// ProjectileMovement drives the sphere via SafeMoveUpdatedComponent (sweep
	// with hit generation). Subclasses tune InitialSpeed / MaxSpeed /
	// ProjectileGravityScale / bShouldBounce in their ctor. We leave all
	// defaults here except RotationFollowsVelocity = true so the mesh +
	// forward vector always point along flight, which also keeps visual
	// orientation consistent with the weapon's aim axis.
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	// Project-wide: the sphere is the thing that gets swept, and OnComponentHit
	// fires as a UFUNCTION callback. Bind in the ctor so BP subclasses inherit
	// the wiring — binding in BeginPlay would work too, but the ctor keeps the
	// binding in one place with the collision setup.
	CollisionSphere->OnComponentHit.AddDynamic(this, &AQuakeProjectile::OnSphereHit);
}

void AQuakeProjectile::BeginPlay()
{
	Super::BeginPlay();

	// SPEC 1.6 rule 1 second guard: never collide with the firing pawn, even
	// if the weapon forgot the 60 u muzzle spawn-out. This also covers the
	// case where the projectile grazes back through the firer on a corner.
	if (AActor* Firer = GetInstigator())
	{
		if (CollisionSphere)
		{
			CollisionSphere->IgnoreActorWhenMoving(Firer, /*bShouldIgnore*/ true);
		}
	}
}

void AQuakeProjectile::OnSphereHit(
	UPrimitiveComponent* /*HitComponent*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	FVector /*NormalImpulse*/,
	const FHitResult& Hit)
{
	// Extra safety net: never detonate on the firer. The IgnoreActorWhenMoving
	// call in BeginPlay should already filter this at the sweep level, but
	// the explicit check costs nothing and documents the invariant.
	if (OtherActor && OtherActor == GetInstigator())
	{
		return;
	}
	HandleImpact(Hit, OtherActor);
}

void AQuakeProjectile::HandleImpact(const FHitResult& /*Hit*/, AActor* /*OtherActor*/)
{
	// Default: no-op. Concrete projectiles (Rocket, Nail, Grenade) override.
}
