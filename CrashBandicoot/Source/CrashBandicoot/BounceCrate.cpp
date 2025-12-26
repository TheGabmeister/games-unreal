#include "BounceCrate.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CBCharacter.h"
#include "Kismet/GameplayStatics.h"

ABounceCrate::ABounceCrate()
{
    PrimaryActorTick.bCanEverTick = false;

    Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
    RootComponent = Collision;
    Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    Collision->SetNotifyRigidBodyCollision(true); // generate hit events

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Collision->OnComponentHit.AddDynamic(this, &ABounceCrate::OnCrateHit);
}

void ABounceCrate::OnCrateHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
    ACBCharacter* Character = Cast<ACBCharacter>(OtherActor);
    if (!Character)
    {
        return;
    }

    // Only bounce if the character hits the TOP of the crate
    const FVector ImpactNormal = Hit.ImpactNormal;
    if (ImpactNormal.Z < 0.7f) // mostly‑upwards normal
    {
        UE_LOG(LogTemp, Warning, TEXT("Hello World"));
        return;
    }

    // Launch character upwards, keep horizontal speed
    Character->LaunchCharacter(FVector(0.f, 0.f, BounceStrength), false, true);

    if (BounceSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
    }

    // Optional: break / destroy box here
    // Destroy();
}