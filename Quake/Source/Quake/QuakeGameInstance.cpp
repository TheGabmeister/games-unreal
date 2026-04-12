#include "QuakeGameInstance.h"

#include "QuakeCharacter.h"
#include "QuakeWeaponBase.h"

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
