#include "Game/CBGameMode.h"
#include "Game/CBGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

ACBGameMode::ACBGameMode()
{
}

void ACBGameMode::BeginPlay()
{
    Super::BeginPlay();
}

void ACBGameMode::PlayerDied()
{
    GetWorldTimerManager().SetTimer(TimerHandle_Restart, this, &ACBGameMode::HandleGameOver, RestartDelay);
}

void ACBGameMode::HandleGameOver()
{
    UCBGameInstance* GI = Cast<UCBGameInstance>(UGameplayStatics::GetGameInstance(this));
    if (!GI)
    {
        return;
    }

    if (GI->LoseLife())
    {
        ResetCurrentLevel();
    }
    else
    {
        UGameplayStatics::OpenLevel(this, TEXT("GameOver"));
    }
}

void ACBGameMode::ResetCurrentLevel()
{
    UWorld* World = GetWorld();

    FString LevelName = World->GetMapName();
    LevelName.RemoveFromStart(World->StreamingLevelsPrefix);

    FWorldContext& WorldContext = GEngine->GetWorldContextFromWorldChecked(World);
    FURL TestURL(&WorldContext.LastURL, *LevelName, TRAVEL_Absolute);

    if (TestURL.IsLocalInternal())
    {
        if (!GEngine->MakeSureMapNameIsValid(TestURL.Map))
        {
            UE_LOG(LogLevel, Warning, TEXT("WARNING: The map '%s' does not exist."), *TestURL.Map);
        }
    }

    GEngine->SetClientTravel(World, *LevelName, TRAVEL_Absolute);
}
