#include "QuakeGameMode.h"

#include "QuakeCharacter.h"
#include "QuakeEnemySpawnPoint.h"
#include "QuakeGameInstance.h"
#include "QuakeHUD.h"
#include "QuakePickupBase.h"
#include "QuakePlayerController.h"
#include "QuakePlayerState.h"
#include "QuakeSaveArchive.h"
#include "QuakeSaveGame.h"
#include "QuakeSaveable.h"
#include "QuakeTrigger_Secret.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeGameMode, Log, All);

AQuakeGameMode::AQuakeGameMode()
{
	DefaultPawnClass = AQuakeCharacter::StaticClass();
	PlayerControllerClass = AQuakePlayerController::StaticClass();
	PlayerStateClass = AQuakePlayerState::StaticClass();
	HUDClass = AQuakeHUD::StaticClass();

	// DESIGN 6.1 default multipliers. BP_QuakeGameMode can override any row.
	// An unconfigured BP inherits these defaults via the C++ constructor.
	{
		FQuakeDifficultyMultipliers Easy;
		Easy.EnemyDamage = 0.75f;
		Easy.EnemyHP     = 1.0f;
		DifficultyTable.Add(EQuakeDifficulty::Easy, Easy);
	}
	{
		FQuakeDifficultyMultipliers Normal;
		DifficultyTable.Add(EQuakeDifficulty::Normal, Normal);
	}
	{
		FQuakeDifficultyMultipliers Hard;
		Hard.EnemyDamage = 1.5f;
		Hard.EnemyHP     = 1.25f;
		DifficultyTable.Add(EQuakeDifficulty::Hard, Hard);
	}
	{
		FQuakeDifficultyMultipliers Nightmare;
		Nightmare.EnemyDamage      = 2.0f;
		Nightmare.EnemyHP          = 1.5f;
		Nightmare.bSuppressPain    = true;
		Nightmare.ZombieReviveScale = 0.5f;
		DifficultyTable.Add(EQuakeDifficulty::Nightmare, Nightmare);
	}
}

EQuakeDifficulty AQuakeGameMode::GetDifficulty() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const UQuakeGameInstance* GI = World->GetGameInstance<UQuakeGameInstance>())
		{
			return GI->GetDifficulty();
		}
	}
	return EQuakeDifficulty::Normal;
}

FQuakeDifficultyMultipliers AQuakeGameMode::GetDifficultyMultipliers() const
{
	return LookupMultipliers(DifficultyTable, GetDifficulty());
}

FQuakeDifficultyMultipliers AQuakeGameMode::LookupMultipliers(
	const TMap<EQuakeDifficulty, FQuakeDifficultyMultipliers>& Table,
	EQuakeDifficulty Difficulty)
{
	if (const FQuakeDifficultyMultipliers* Found = Table.Find(Difficulty))
	{
		return *Found;
	}
	return FQuakeDifficultyMultipliers{};
}

bool AQuakeGameMode::ShouldRouteToWinScreen(bool bIsFinal, FName NextMapName)
{
	// Final-level exit always routes to win screen, regardless of NextMapName.
	// Non-final + missing NextMap is an authoring bug, not a win — fall through
	// to normal handling so the existing "no NextMap" warning fires.
	return bIsFinal;
}

void AQuakeGameMode::RequestRestartFromDeath(AQuakePlayerController* PC)
{
	UWorld* World = GetWorld();
	if (!World || !PC)
	{
		return;
	}

	// DESIGN 6.4 step 2: clear per-life state (powerups + keys). Kills,
	// Secrets, Time, Deaths persist — they're score, not life-bound.
	if (AQuakePlayerState* PS = PC->GetPlayerState<AQuakePlayerState>())
	{
		PS->ClearPerLifeState();
	}

	// DESIGN 6.4 step 3: restore inventory snapshot.
	if (UQuakeGameInstance* GI = World->GetGameInstance<UQuakeGameInstance>())
	{
		GI->RestoreFromLevelEntrySnapshot();
	}

	// DESIGN 6.4 step 4: destroy dead pawn, spawn fresh at PlayerStart. The
	// engine's RestartPlayer path takes care of the spawn + possess.
	if (APawn* OldPawn = PC->GetPawn())
	{
		PC->UnPossess();
		OldPawn->Destroy();
	}
	RestartPlayer(PC);
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

	// Phase 11: cache the initial-level pickup FName set so save-time can
	// compute `Consumed = Initial \ Live` without authoring per-pickup records.
	InitialPickupNames.Reset();
	for (TActorIterator<AQuakePickupBase> It(GetWorld()); It; ++It)
	{
		InitialPickupNames.Add(It->GetFName());
	}

	// Phase 11: a pending load set up by UQuakeGameInstance::LoadFromSlot
	// needs the new level's actors restored to their saved state. If no
	// pending load, auto-save at level entry (DESIGN 6.2: "auto-save on level
	// entry after snapshot taken") so a fresh level starts with a recoverable
	// slot.
	if (UQuakeGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UQuakeGameInstance>() : nullptr)
	{
		if (UQuakeSaveGame* Pending = GI->ConsumePendingLoad())
		{
			RestoreWorldFromSave(*Pending);
		}
		else
		{
			GI->SaveCurrentState(UQuakeGameInstance::BuildAutoSlotName());
		}

		// DESIGN 6.4 step 3: capture the level-entry snapshot AFTER any save
		// restore so death-restart returns to "what the player walked in
		// with this attempt", not the prior level's exit inventory.
		GI->SnapshotForLevelEntry();
	}
}

void AQuakeGameMode::CaptureWorldSnapshot(UQuakeSaveGame& Out) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	Out.Difficulty       = GetDifficulty();
	Out.CurrentLevelName = UGameplayStatics::GetCurrentLevelName(World, /*bRemovePrefixString*/ true);

	// Pawn transform + HP.
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (AQuakeCharacter* Char = PC ? Cast<AQuakeCharacter>(PC->GetPawn()) : nullptr)
	{
		Out.PlayerTransform = Char->GetActorTransform();
		Out.Health = Char->GetHealth();
	}

	// PlayerState snapshot.
	if (AQuakePlayerState* PS = PC ? PC->GetPlayerState<AQuakePlayerState>() : nullptr)
	{
		PS->CaptureToSave(Out);
	}

	// Per-actor IQuakeSaveable records. TActorIterator<IQuakeSaveable>
	// doesn't exist — iterate AActor and gate on Implements<UQuakeSaveable>.
	TArray<FName> LivePickupNames;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor)
		{
			continue;
		}
		if (Actor->IsA(AQuakePickupBase::StaticClass()))
		{
			LivePickupNames.Add(Actor->GetFName());
		}
		if (Actor->Implements<UQuakeSaveable>())
		{
			FActorSaveRecord Rec;
			if (IQuakeSaveable* Saveable = Cast<IQuakeSaveable>(Actor))
			{
				Saveable->SaveState(Rec);
				if (!Rec.ActorName.IsNone())
				{
					Out.ActorRecords.Add(MoveTemp(Rec));
				}
			}
		}
	}

	Out.ConsumedPickupNames = QuakeSaveArchive::ComputeConsumedNames(
		InitialPickupNames, LivePickupNames);
}

void AQuakeGameMode::RestoreWorldFromSave(UQuakeSaveGame& Save)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);

	// PlayerState — Kills/Secrets/Deaths/ActivePowerups/Keys + time base.
	if (AQuakePlayerState* PS = PC ? PC->GetPlayerState<AQuakePlayerState>() : nullptr)
	{
		PS->ApplyFromSave(Save, World->GetTimeSeconds());
	}

	// Difficulty lives on the GameInstance so it survives OpenLevel. The
	// LoadFromSlot path restores inventory fields BEFORE OpenLevel, but
	// Difficulty is small enough to restore here too — keeps the save
	// schema readable (one section per subsystem).
	if (UQuakeGameInstance* GI = World->GetGameInstance<UQuakeGameInstance>())
	{
		GI->SetDifficulty(Save.Difficulty);
	}

	// Pawn transform.
	AQuakeCharacter* Char = PC ? Cast<AQuakeCharacter>(PC->GetPawn()) : nullptr;
	if (Char)
	{
		Char->SetActorTransform(Save.PlayerTransform, /*bSweep*/ false, nullptr,
			ETeleportType::TeleportPhysics);
		if (AController* Ctrl = Char->GetController())
		{
			Ctrl->SetControlRotation(Save.PlayerTransform.Rotator());
		}
	}

	// Per-actor records keyed by FName.
	TMap<FName, const FActorSaveRecord*> RecordByName;
	RecordByName.Reserve(Save.ActorRecords.Num());
	for (const FActorSaveRecord& Rec : Save.ActorRecords)
	{
		RecordByName.Add(Rec.ActorName, &Rec);
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || !Actor->Implements<UQuakeSaveable>())
		{
			continue;
		}
		if (const FActorSaveRecord* const* Found = RecordByName.Find(Actor->GetFName()))
		{
			if (IQuakeSaveable* Saveable = Cast<IQuakeSaveable>(Actor))
			{
				Saveable->LoadState(**Found);
			}
		}
	}

	// Destroy pickups that were consumed before the save — the fresh level
	// re-spawned them at BeginPlay.
	if (Save.ConsumedPickupNames.Num() > 0)
	{
		for (TActorIterator<AQuakePickupBase> It(World); It; ++It)
		{
			if (Save.ConsumedPickupNames.Contains(It->GetFName()))
			{
				It->Destroy();
			}
		}
	}

	UE_LOG(LogQuakeGameMode, Log,
		TEXT("RestoreWorldFromSave: %d records, %d consumed pickups, level=%s"),
		Save.ActorRecords.Num(), Save.ConsumedPickupNames.Num(), *Save.CurrentLevelName);
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
