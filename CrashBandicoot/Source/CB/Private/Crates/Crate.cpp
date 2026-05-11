// Crate.cpp
#include "Crates/Crate.h"
#include "Pickups/WumpaFruit.h"
#include "Pickups/AkuAkuPickup.h"
#include "Pickups/LifePickup.h"

void ACrate::SpawnContents()
{
    FVector SpawnLoc = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    switch (Contents)
    {
    case ECrateContents::Wumpa:
    case ECrateContents::WumpaLarge:
    {
        int32 Count = (Contents == ECrateContents::WumpaLarge)
            ? FMath::RandRange(5, 10)
            : (FMath::RandBool() ? 1 : 2);
        for (int32 i = 0; i < Count; ++i)
        {
            FVector Offset(FMath::RandRange(-50.0f, 50.0f), FMath::RandRange(-50.0f, 50.0f), FMath::RandRange(0.0f, 80.0f));
            GetWorld()->SpawnActor<AWumpaFruit>(AWumpaFruit::StaticClass(), SpawnLoc + Offset, FRotator::ZeroRotator, Params);
        }
        break;
    }
    case ECrateContents::Life:
        GetWorld()->SpawnActor<ALifePickup>(ALifePickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
        break;
    case ECrateContents::Mask:
        GetWorld()->SpawnActor<AAkuAkuPickup>(AAkuAkuPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
        break;
    case ECrateContents::Token:
        // Phase 8 stub — no token actor yet
        break;
    }
}
