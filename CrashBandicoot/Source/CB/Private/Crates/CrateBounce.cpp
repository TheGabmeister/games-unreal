// CrateBounce.cpp
#include "Crates/CrateBounce.h"
#include "Player/CBPlayerCharacter.h"
#include "Pickups/WumpaFruit.h"
#include "Kismet/GameplayStatics.h"

void ACrateBounce::OnJumpHit(ACBPlayerCharacter* Player)
{
    bHitThisFrame = true;
    BounceCount++;

    SpawnSingleWumpa();

    if (BounceSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
    }

    // Bounce the player using the player's default stomp bounce
    if (Player)
    {
        Player->LaunchCharacter(FVector(0.0f, 0.0f, Player->StompBounceVelocity), false, true);
    }

    if (BounceCount >= MaxBounces)
    {
        BreakCrate();
    }
}

void ACrateBounce::OnSpinHit(ACBPlayerCharacter* Player)
{
    if (bHitThisFrame) return;
    BreakCrate();
}

void ACrateBounce::SpawnContents()
{
    SpawnSingleWumpa();
}

void ACrateBounce::SpawnSingleWumpa()
{
    FVector SpawnLoc = GetActorLocation() + FVector(0.0f, 0.0f, 80.0f);
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    GetWorld()->SpawnActor<AWumpaFruit>(AWumpaFruit::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
}
