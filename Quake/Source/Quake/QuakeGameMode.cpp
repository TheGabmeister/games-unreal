#include "QuakeGameMode.h"

#include "QuakeCharacter.h"
#include "QuakeEnemySpawnPoint.h"
#include "QuakeHUD.h"
#include "QuakePlayerController.h"
#include "QuakePlayerState.h"
#include "QuakeTrigger_Secret.h"

#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeGameMode, Log, All);

AQuakeGameMode::AQuakeGameMode()
{
	DefaultPawnClass = AQuakeCharacter::StaticClass();
	PlayerControllerClass = AQuakePlayerController::StaticClass();
	PlayerStateClass = AQuakePlayerState::StaticClass();
	HUDClass = AQuakeHUD::StaticClass();
}

void AQuakeGameMode::BeginPlay()
{
	Super::BeginPlay();

	// SPEC 5.9: compute denominators once. Spawn points count regardless of
	// bDeferredSpawn — a deferred point still owes an enemy to the level
	// total. Directly-placed BP_Enemy_* actors are intentionally ignored.
	KillsTotal = 0;
	for (TActorIterator<AQuakeEnemySpawnPoint> It(GetWorld()); It; ++It)
	{
		if (It->IsEligible())
		{
			++KillsTotal;
		}
	}

	SecretsTotal = 0;
	for (TActorIterator<AQuakeTrigger_Secret> It(GetWorld()); It; ++It)
	{
		++SecretsTotal;
	}

	UE_LOG(LogQuakeGameMode, Log,
		TEXT("Level totals: KillsTotal=%d, SecretsTotal=%d"), KillsTotal, SecretsTotal);
}

bool AQuakeGameMode::IsLevelCleared() const
{
	for (TActorIterator<AQuakeEnemySpawnPoint> It(GetWorld()); It; ++It)
	{
		if (It->IsEligible() && !It->IsSatisfied())
		{
			return false;
		}
	}
	return true;
}

bool AQuakeGameMode::IsLevelClearedForSet(const TArray<const AQuakeEnemySpawnPoint*>& SpawnPoints)
{
	for (const AQuakeEnemySpawnPoint* SP : SpawnPoints)
	{
		if (SP && SP->IsEligible() && !SP->IsSatisfied())
		{
			return false;
		}
	}
	return true;
}
