// CrateIronArrow.cpp
#include "Crates/CrateIronArrow.h"
#include "Player/CBPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

ACrateIronArrow::ACrateIronArrow()
{
    bCountsTowardTotal = false;
}

void ACrateIronArrow::OnJumpHit(ACBPlayerCharacter* Player)
{
    if (ACharacter* Char = Cast<ACharacter>(Player))
    {
        Char->LaunchCharacter(FVector(0.0f, 0.0f, LaunchVelocity), false, true);
    }

    if (LaunchSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, LaunchSound, GetActorLocation());
    }
    // Does NOT break
}
