// CrateArrow.cpp
#include "Crates/CrateArrow.h"
#include "Player/CBPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void ACrateArrow::OnJumpHit(ACBPlayerCharacter* Player)
{
    if (ACharacter* Char = Cast<ACharacter>(Player))
    {
        Char->LaunchCharacter(FVector(0.0f, 0.0f, LaunchVelocity), false, true);
    }

    if (LaunchSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, LaunchSound, GetActorLocation());
    }
}

void ACrateArrow::OnSpinHit(ACBPlayerCharacter* Player)
{
    BreakCrate();
}
