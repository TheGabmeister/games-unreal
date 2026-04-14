#include "QuakeSoundManager.h"

#include "QuakeGameInstance.h"

#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeSound, Log, All);

void UQuakeSoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bTableResolved = false;
	CachedTable = nullptr;
}

void UQuakeSoundManager::Deinitialize()
{
	CachedTable = nullptr;
	bTableResolved = false;
	Super::Deinitialize();
}

UDataTable* UQuakeSoundManager::ResolveTable()
{
	if (bTableResolved)
	{
		return CachedTable;
	}
	bTableResolved = true;

	UQuakeGameInstance* GI = Cast<UQuakeGameInstance>(GetGameInstance());
	if (!GI)
	{
		return nullptr;
	}
	CachedTable = GI->SoundEventTable.LoadSynchronous();
	return CachedTable;
}

FName UQuakeSoundManager::ResolveRowName(EQuakeSoundEvent Event)
{
	const UEnum* EnumPtr = StaticEnum<EQuakeSoundEvent>();
	if (!EnumPtr)
	{
		return NAME_None;
	}
	return FName(*EnumPtr->GetNameStringByValue(static_cast<int64>(Event)));
}

const FQuakeSoundEvent* UQuakeSoundManager::FindRow(EQuakeSoundEvent Event)
{
	UDataTable* Table = ResolveTable();
	if (!Table)
	{
		return nullptr;
	}
	const FName RowName = ResolveRowName(Event);
	if (RowName.IsNone())
	{
		return nullptr;
	}
	return Table->FindRow<FQuakeSoundEvent>(RowName, TEXT("UQuakeSoundManager::FindRow"), /*bWarnIfMissing*/ false);
}

void UQuakeSoundManager::PlaySound(EQuakeSoundEvent Event, FVector Location)
{
	if (Event == EQuakeSoundEvent::None)
	{
		return;
	}

	UE_LOG(LogQuakeSound, Verbose, TEXT("PlaySound %s @ (%.0f,%.0f,%.0f)"),
		*UEnum::GetValueAsString(Event), Location.X, Location.Y, Location.Z);

	const FQuakeSoundEvent* Row = FindRow(Event);
	if (!Row || !Row->Sound)
	{
		// Authored row missing or asset unassigned — Phase 14 gameplay
		// stub. The Verbose log above is the only feedback.
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	UGameplayStatics::PlaySoundAtLocation(World, Row->Sound, Location,
		Row->VolumeMultiplier, Row->PitchMultiplier);
}

void UQuakeSoundManager::PlaySound2D(EQuakeSoundEvent Event)
{
	if (Event == EQuakeSoundEvent::None)
	{
		return;
	}

	UE_LOG(LogQuakeSound, Verbose, TEXT("PlaySound2D %s"), *UEnum::GetValueAsString(Event));

	const FQuakeSoundEvent* Row = FindRow(Event);
	if (!Row || !Row->Sound)
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	UGameplayStatics::PlaySound2D(World, Row->Sound,
		Row->VolumeMultiplier, Row->PitchMultiplier);
}

void UQuakeSoundManager::PlayMusic(EQuakeMusicTrack Track)
{
	UE_LOG(LogQuakeSound, Verbose, TEXT("PlayMusic %s (stub)"),
		*UEnum::GetValueAsString(Track));
}

void UQuakeSoundManager::StopMusic()
{
	UE_LOG(LogQuakeSound, Verbose, TEXT("StopMusic (stub)"));
}

UQuakeSoundManager* UQuakeSoundManager::Get(const UObject* WorldContext)
{
	if (!WorldContext)
	{
		return nullptr;
	}
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World)
	{
		return nullptr;
	}
	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UQuakeSoundManager>() : nullptr;
}

void UQuakeSoundManager::PlaySoundEvent(const UObject* WorldContext, EQuakeSoundEvent Event, const FVector& Location)
{
	if (UQuakeSoundManager* Mgr = Get(WorldContext))
	{
		Mgr->PlaySound(Event, Location);
	}
}
