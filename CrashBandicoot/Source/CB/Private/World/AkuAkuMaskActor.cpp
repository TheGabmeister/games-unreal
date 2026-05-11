#include "World/AkuAkuMaskActor.h"
#include "Components/StaticMeshComponent.h"

AAkuAkuMaskActor::AAkuAkuMaskActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAkuAkuMaskActor::SetNormalAppearance()
{
    if (NormalMaterial)
    {
        MeshComponent->SetMaterial(0, NormalMaterial);
    }
}

void AAkuAkuMaskActor::SetGoldenAppearance()
{
    if (GoldenMaterial)
    {
        MeshComponent->SetMaterial(0, GoldenMaterial);
    }
}
