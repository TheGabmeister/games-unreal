// PickupBase.cpp
#include "Pickups/PickupBase.h"
#include "CBCollisionChannels.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CBPlayerCharacter.h"

APickupBase::APickupBase()
{
    PrimaryActorTick.bCanEverTick = true;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    PickupTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("PickupTrigger"));
    PickupTrigger->SetupAttachment(RootComponent);
    PickupTrigger->SetSphereRadius(50.0f);
    PickupTrigger->SetCollisionObjectType(CBCollision::Pickup);
    PickupTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupTrigger->SetCollisionResponseToChannel(CBCollision::Player, ECR_Overlap);
    PickupTrigger->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap); // Default pawn channel
    PickupTrigger->SetGenerateOverlapEvents(true);
}

void APickupBase::BeginPlay()
{
    Super::BeginPlay();
    PickupTrigger->OnComponentBeginOverlap.AddDynamic(this, &APickupBase::OnPickupOverlap);
}

void APickupBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (RotationSpeed != 0.0f)
    {
        AddActorLocalRotation(FRotator(0.0f, RotationSpeed * DeltaTime, 0.0f));
    }
}

void APickupBase::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACBPlayerCharacter* Player = Cast<ACBPlayerCharacter>(OtherActor);
    if (Player)
    {
        Collect(Player);
    }
}

void APickupBase::Collect(ACBPlayerCharacter* Player)
{
    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
    }
    OnPickedUp(Player);
    Destroy();
}

void APickupBase::OnPickedUp(ACBPlayerCharacter* Player)
{
    // Override in subclasses
}
