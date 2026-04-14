#include "QuakePickupBase.h"

#include "QuakeCharacter.h"
#include "QuakeCollisionChannels.h"
#include "QuakeSoundManager.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakePickup, Log, All);

AQuakePickupBase::AQuakePickupBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Sphere is the root so the visual mesh and light can be offset /
	// scaled without moving the overlap volume.
	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->InitSphereRadius(64.f);  // SPEC section 4 default.
	RootComponent = PickupSphere;

	// SPEC 1.6 object response matrix + rule 4: pickup sphere uses the
	// custom Pickup object channel, overlap-only. Ignores everything except
	// Pawn, which it overlaps so the player's capsule triggers the
	// BeginOverlap. Per rule 4 we then filter to AQuakeCharacter in the
	// handler so enemies can touch pickups without consuming them.
	PickupSphere->SetCollisionObjectType(QuakeCollision::ECC_Pickup);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupSphere->SetGenerateOverlapEvents(true);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(PickupSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetCastShadow(false);

	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(PickupSphere);
	GlowLight->SetIntensity(10000.f);
	GlowLight->SetAttenuationRadius(200.f);
	GlowLight->SetCastShadows(false);
}

void AQuakePickupBase::BeginPlay()
{
	Super::BeginPlay();

	if (PickupSphere)
	{
		PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuakePickupBase::OnPickupBeginOverlap);
	}
}

void AQuakePickupBase::OnPickupBeginOverlap(
	UPrimitiveComponent* /*OverlappedComponent*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/,
	const FHitResult& /*SweepResult*/)
{
	// Player-only filter per SPEC 1.6 rule 4. Enemies trigger the overlap
	// but never reach CanBeConsumedBy / ApplyPickupEffectTo.
	AQuakeCharacter* Character = Cast<AQuakeCharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	if (!CanBeConsumedBy(Character))
	{
		// "Not permitted right now" — e.g., full-HP player touching a
		// non-overcharge health pickup. Leave the pickup in the world so
		// they can come back for it later. SPEC 4.1 "health is only
		// consumed if Health < Max".
		return;
	}

	ApplyPickupEffectTo(Character);

	UE_LOG(LogQuakePickup, Verbose, TEXT("%s consumed by %s"), *GetName(), *Character->GetName());

	UQuakeSoundManager::PlaySoundEvent(this, GetPickupSoundEvent(), GetActorLocation());

	// Destroy immediately. Rotation / bob visuals fall away with the actor.
	Destroy();
}
