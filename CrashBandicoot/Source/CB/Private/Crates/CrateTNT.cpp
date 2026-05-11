// CrateTNT.cpp
#include "Crates/CrateTNT.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Character.h"
#include "Player/CBPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/InteractionInterfaces.h"
#include "TimerManager.h"

ACrateTNT::ACrateTNT()
{
    CountdownText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CountdownText"));
    CountdownText->SetupAttachment(MeshComponent);
    CountdownText->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
    CountdownText->SetHorizontalAlignment(EHTA_Center);
    CountdownText->SetVerticalAlignment(EVRTA_TextCenter);
    CountdownText->SetWorldSize(60.0f);
    CountdownText->SetText(FText::GetEmpty());
}

void ACrateTNT::OnSpinHit(ACBPlayerCharacter* Player)
{
    Detonate();
}

void ACrateTNT::OnJumpHit(ACBPlayerCharacter* Player)
{
    if (Player)
    {
        Player->LaunchCharacter(FVector(0.0f, 0.0f, Player->StompBounceVelocity), false, true);
    }

    if (!bCountdownStarted)
    {
        StartCountdown();
    }
}

void ACrateTNT::OnExplosionHit(FVector Origin, float Radius)
{
    // Chain detonation with slight delay
    if (!bDetonated)
    {
        FTimerHandle ChainTimer;
        GetWorldTimerManager().SetTimer(ChainTimer, this, &ACrateTNT::Detonate, ChainDetonationDelay);
    }
}

void ACrateTNT::StartCountdown()
{
    bCountdownStarted = true;
    CountdownValue = 3;
    CountdownText->SetText(FText::AsNumber(CountdownValue));

    if (TickSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, TickSound, GetActorLocation());
    }

    GetWorldTimerManager().SetTimer(TimerHandle_Countdown, this, &ACrateTNT::TickCountdown, 1.0f, true);
}

void ACrateTNT::TickCountdown()
{
    CountdownValue--;

    if (CountdownValue <= 0)
    {
        GetWorldTimerManager().ClearTimer(TimerHandle_Countdown);
        Detonate();
        return;
    }

    CountdownText->SetText(FText::AsNumber(CountdownValue));

    if (TickSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, TickSound, GetActorLocation());
    }
}

void ACrateTNT::Detonate()
{
    if (bDetonated) return;
    bDetonated = true;

    GetWorldTimerManager().ClearTimer(TimerHandle_Countdown);

    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
    }

    // Sphere overlap for blast radius
    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(this, GetActorLocation(), BlastRadius,
        ObjectTypes, nullptr, TArray<AActor*>{this}, OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (IExplodable* Explodable = Cast<IExplodable>(Actor))
        {
            Explodable->OnExplosionHit(GetActorLocation(), BlastRadius);
        }
    }

    // Damage player if in range
    if (ACBPlayerCharacter* Player = Cast<ACBPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
    {
        float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
        if (Dist <= BlastRadius)
        {
            Player->OnHit(this);
        }
    }

    Destroy();
}
