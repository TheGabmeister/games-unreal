#include "QuakeGameInstance.h"

#include "QuakeCharacter.h"
#include "QuakeGameMode.h"
#include "QuakeInventoryComponent.h"
#include "QuakeSaveGame.h"
#include "QuakeWeaponBase.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeSave, Log, All);

UQuakeGameInstance* UQuakeGameInstance::GetChecked(const UObject* WorldContext)
{
	// UObject* overload rather than UWorld* so callers don't have to unwrap
	// GetWorld() themselves — matches the shape of other engine helpers.
	UWorld* World = GEngine
		? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)
		: nullptr;
	UGameInstance* Raw = World ? World->GetGameInstance() : nullptr;
	UQuakeGameInstance* GI = Cast<UQuakeGameInstance>(Raw);
	checkf(GI != nullptr,
		TEXT("UQuakeGameInstance::GetChecked: GameInstance is %s — set GameInstanceClass "
		     "to BP_QuakeGameInstance (or UQuakeGameInstance) in Project Settings > Maps & Modes."),
		Raw ? *Raw->GetClass()->GetName() : TEXT("null"));
	return GI;
}

UQuakeGameInstance::UQuakeGameInstance() = default;

void UQuakeGameInstance::Init()
{
	Super::Init();
	// Inventory lives on UQuakeInventoryComponent; no GI-side seeding.
}

// --- Phase 11: save/load ---

FString UQuakeGameInstance::BuildAutoSlotName()
{
	// Profile field is placeholder until Phase 13 wires a profile UI. DESIGN 6.2
	// prescribes `auto_<profile>` / `quick_<profile>` as the slot schema.
	return TEXT("auto_default");
}

FString UQuakeGameInstance::BuildQuickSlotName()
{
	return TEXT("quick_default");
}

void UQuakeGameInstance::CaptureInventorySnapshot(UQuakeSaveGame& Out) const
{
	// Resolve the live player's component and serialize directly. No live
	// inventory fields on the GI post-refactor — if there's no pawn (e.g.
	// a save triggered pre-pawn), write an invalid snapshot and let the
	// next load fall back to UPROPERTY defaults.
	const UWorld* World = GetWorld();
	APlayerController* PC = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
	const AQuakeCharacter* Char = PC ? Cast<AQuakeCharacter>(PC->GetPawn()) : nullptr;
	const UQuakeInventoryComponent* Comp = Char ? Char->GetInventoryComponent() : nullptr;

	if (Comp)
	{
		Comp->SerializeTo(Out.InventorySnapshot);
	}
	else
	{
		Out.InventorySnapshot = FQuakeInventorySnapshot{};  // bValid=false
	}
}

void UQuakeGameInstance::ApplyInventorySnapshot(const UQuakeSaveGame& In)
{
	// Queue for the next-spawned pawn's component to consume on
	// InitializeComponent (see UQuakeInventoryComponent::InitializeComponent).
	TransitSnapshot = In.InventorySnapshot;
}

bool UQuakeGameInstance::SaveCurrentState(const FString& SlotName)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogQuakeSave, Warning, TEXT("SaveCurrentState: no World."));
		return false;
	}

	UQuakeSaveGame* Save = Cast<UQuakeSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UQuakeSaveGame::StaticClass()));
	if (!Save)
	{
		return false;
	}

	// 1) Inventory straight from the live pawn's component.
	CaptureInventorySnapshot(*Save);

	// 2) Per-level state: the authoritative GameMode owns the iteration
	//    over IQuakeSaveable actors + PlayerState capture. Keeps the
	//    per-world code in one place and out of the GameInstance.
	if (AQuakeGameMode* GM = World->GetAuthGameMode<AQuakeGameMode>())
	{
		GM->CaptureWorldSnapshot(*Save);
	}

	const bool bOk = UGameplayStatics::SaveGameToSlot(Save, SlotName, /*UserIndex*/ 0);
	UE_LOG(LogQuakeSave, Log, TEXT("SaveCurrentState(%s): %s"),
		*SlotName, bOk ? TEXT("OK") : TEXT("FAILED"));
	return bOk;
}

bool UQuakeGameInstance::LoadFromSlot(const FString& SlotName)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, /*UserIndex*/ 0))
	{
		UE_LOG(LogQuakeSave, Log, TEXT("LoadFromSlot(%s): slot does not exist."), *SlotName);
		return false;
	}

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, /*UserIndex*/ 0);
	UQuakeSaveGame* Save = Cast<UQuakeSaveGame>(Loaded);
	if (!Save)
	{
		UE_LOG(LogQuakeSave, Warning, TEXT("LoadFromSlot(%s): cast to UQuakeSaveGame failed."), *SlotName);
		return false;
	}

	// DESIGN 6.2 step 2: queue the inventory for the next pawn BEFORE OpenLevel
	// so the fresh Character's InventoryComponent::InitializeComponent reads it.
	ApplyInventorySnapshot(*Save);

	// Stash for the new world's GameMode to pick up in BeginPlay.
	PendingLoad = Save;

	// DESIGN 6.2 step 3: open the saved level.
	const FName LevelName(*Save->CurrentLevelName);
	UGameplayStatics::OpenLevel(this, LevelName);
	return true;
}

UQuakeSaveGame* UQuakeGameInstance::ConsumePendingLoad()
{
	UQuakeSaveGame* Out = PendingLoad;
	PendingLoad = nullptr;
	return Out;
}

void UQuakeGameInstance::SnapshotForLevelEntry(const UQuakeInventoryComponent* Comp)
{
	if (Comp)
	{
		Comp->SerializeTo(LevelEntrySnapshot);
	}
}

void UQuakeGameInstance::RestoreFromLevelEntrySnapshot()
{
	if (!LevelEntrySnapshot.bValid)
	{
		return;
	}
	// Queue for the next-spawned pawn's component to consume on
	// InitializeComponent. The pawn has already been destroyed by the caller
	// (RequestRestartFromDeath); the fresh pawn's component hydrates from
	// this mailbox, producing byte-identical inventory to level entry.
	TransitSnapshot = LevelEntrySnapshot;
}
