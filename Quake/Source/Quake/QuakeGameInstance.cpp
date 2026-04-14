#include "QuakeGameInstance.h"

#include "QuakeCharacter.h"
#include "QuakeGameMode.h"
#include "QuakeSaveGame.h"
#include "QuakeWeaponBase.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeSave, Log, All);

UQuakeGameInstance::UQuakeGameInstance()
{
	// SPEC 2.0: 8 weapon slots indexed by their number key (1 = Axe, 2 =
	// Shotgun, ..., 8 = Thunderbolt). Pre-size the array so BP_QuakeGameInstance
	// editor defaults can fill specific indices without worrying about
	// array resizing.
	OwnedWeaponClasses.SetNum(AQuakeCharacter::NumWeaponSlots);
}

void UQuakeGameInstance::Init()
{
	Super::Init();

	// SPEC 1.4 starting loadout: 25 shells. Seed the TMap so GetAmmo works
	// without a prior Give. Other ammo types start at 0.
	AmmoCounts.Add(EQuakeAmmoType::Shells,  25);
	AmmoCounts.Add(EQuakeAmmoType::Nails,   0);
	AmmoCounts.Add(EQuakeAmmoType::Rockets, 0);
	AmmoCounts.Add(EQuakeAmmoType::Cells,   0);
}

int32 UQuakeGameInstance::GetAmmoCap(EQuakeAmmoType Type)
{
	// SPEC section 2.1 ammo table.
	switch (Type)
	{
	case EQuakeAmmoType::Shells:  return 100;
	case EQuakeAmmoType::Nails:   return 200;
	case EQuakeAmmoType::Rockets: return 100;
	case EQuakeAmmoType::Cells:   return 100;
	default:                      return 0;
	}
}

int32 UQuakeGameInstance::GetAmmo(EQuakeAmmoType Type) const
{
	if (Type == EQuakeAmmoType::None)
	{
		return 0;
	}
	const int32* Found = AmmoCounts.Find(Type);
	return Found ? *Found : 0;
}

int32 UQuakeGameInstance::GiveAmmo(EQuakeAmmoType Type, int32 Amount)
{
	if (Type == EQuakeAmmoType::None || Amount <= 0)
	{
		return 0;
	}
	const int32 Cap = GetAmmoCap(Type);
	int32& Current = AmmoCounts.FindOrAdd(Type);
	const int32 Before = Current;
	Current = FMath::Min(Current + Amount, Cap);
	return Current - Before;
}

bool UQuakeGameInstance::ConsumeAmmo(EQuakeAmmoType Type, int32 Amount)
{
	if (Type == EQuakeAmmoType::None)
	{
		// Weapons with no ammo type (Axe) always succeed — they can fire
		// without consuming anything.
		return true;
	}
	if (Amount <= 0)
	{
		return true;
	}
	int32& Current = AmmoCounts.FindOrAdd(Type);
	if (Current < Amount)
	{
		return false;
	}
	Current -= Amount;
	return true;
}

void UQuakeGameInstance::GiveWeapon(int32 SlotNumberOneBased, TSubclassOf<AQuakeWeaponBase> WeaponClass)
{
	const int32 Index = SlotNumberOneBased - 1;
	if (Index < 0 || Index >= OwnedWeaponClasses.Num())
	{
		return;
	}
	OwnedWeaponClasses[Index] = WeaponClass;
}

bool UQuakeGameInstance::OwnsWeaponInSlot(int32 SlotIndexZeroBased) const
{
	if (SlotIndexZeroBased < 0 || SlotIndexZeroBased >= OwnedWeaponClasses.Num())
	{
		return false;
	}
	return OwnedWeaponClasses[SlotIndexZeroBased] != nullptr;
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
	Out.Armor              = Armor;
	Out.ArmorAbsorption    = ArmorAbsorption;
	Out.OwnedWeaponClasses = OwnedWeaponClasses;
	Out.AmmoCounts         = AmmoCounts;
}

void UQuakeGameInstance::ApplyInventorySnapshot(const UQuakeSaveGame& In)
{
	Armor              = In.Armor;
	ArmorAbsorption    = In.ArmorAbsorption;
	OwnedWeaponClasses = In.OwnedWeaponClasses;

	// Pre-seed keys so GetAmmo without a prior Give still works. Copy the
	// saved counts, then union in any missing keys so downstream code can
	// rely on all four ammo types being present.
	AmmoCounts = In.AmmoCounts;
	for (EQuakeAmmoType Type : { EQuakeAmmoType::Shells, EQuakeAmmoType::Nails,
								  EQuakeAmmoType::Rockets, EQuakeAmmoType::Cells })
	{
		AmmoCounts.FindOrAdd(Type);
	}
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

	// 1) Inventory + profile straight from the GameInstance.
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

	// DESIGN 6.2 step 2: restore GameInstance fields BEFORE OpenLevel so
	// the fresh Character's BeginPlay reads the restored inventory.
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
