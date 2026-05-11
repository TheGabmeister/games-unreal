// CrateBase.cpp
#include "Crates/CrateBase.h"
#include "Player/CBPlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ACrateBase::ACrateBase()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
}

void ACrateBase::OnSpinHit(ACBPlayerCharacter* Player) { BreakCrate(); }
void ACrateBase::OnJumpHit(ACBPlayerCharacter* Player)
{
    if (Player)
    {
        Player->LaunchCharacter(FVector(0.0f, 0.0f, Player->StompBounceVelocity), false, true);
    }
    BreakCrate();
}
void ACrateBase::OnExplosionHit(FVector Origin, float Radius) { BreakCrate(); }

void ACrateBase::BreakCrate()
{
    if (bIsBroken) return;
    bIsBroken = true;

    if (BreakSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BreakSound, GetActorLocation());
    }

    SpawnContents();
    Destroy();
}

void ACrateBase::SpawnContents()
{
    // Override in subclasses
}
