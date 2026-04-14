#include "QuakeInventoryComponent.h"

#include "QuakeCharacter.h"
#include "QuakeGameInstance.h"
#include "QuakeWeaponBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeInventory, Log, All);

UQuakeInventoryComponent::UQuakeInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// InitializeComponent (not BeginPlay) so hydration happens PRE-BeginPlay
	// across the whole world. GameMode::BeginPlay's level-entry auto-save +
	// SnapshotForLevelEntry both call into the component; by the time any
	// BeginPlay fires, the component is already hydrated from the mailbox.
	bWantsInitializeComponent = true;

	// SPEC 2.0: 8 weapon slots indexed by their number key (1 = Axe, ..., 8 =
	// Thunderbolt). Pre-size so BP_QuakeCharacter editor defaults can fill
	// specific indices without worrying about array resizing.
	OwnedWeaponClasses.SetNum(AQuakeCharacter::NumWeaponSlots);
}

void UQuakeInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();

	UQuakeGameInstance* GI = UQuakeGameInstance::GetChecked(this);

	if (GI->TransitSnapshot.bValid)
	{
		// Consume the mailbox: level-entry handoff from the previous pawn
		// (level transition, save-load, or death-restart).
		DeserializeFrom(GI->TransitSnapshot);
		GI->TransitSnapshot.bValid = false;
	}
	else
	{
		// First-run path (new game, no save, no prior pawn). Use UPROPERTY
		// defaults (OwnedWeaponClasses authored on BP_QuakeCharacter) plus
		// the starting-ammo seed.
		SeedDefaultsIfEmpty();
	}
}

void UQuakeInventoryComponent::SeedDefaultsIfEmpty()
{
	// SPEC 1.4 starting loadout: 25 shells (tunable via StartingShells on
	// BP_QuakeCharacter). Other ammo types start at 0.
	AmmoCounts.FindOrAdd(EQuakeAmmoType::Shells)  = StartingShells;
	AmmoCounts.FindOrAdd(EQuakeAmmoType::Nails)   = 0;
	AmmoCounts.FindOrAdd(EQuakeAmmoType::Rockets) = 0;
	AmmoCounts.FindOrAdd(EQuakeAmmoType::Cells)   = 0;

	// Dev warning: no weapons authored. The player will spawn empty-handed
	// until BP_QuakeCharacter's InventoryComponent->OwnedWeaponClasses is
	// filled (Axe at minimum).
	bool bAnyWeapon = false;
	for (const TSubclassOf<AQuakeWeaponBase>& Slot : OwnedWeaponClasses)
	{
		if (Slot) { bAnyWeapon = true; break; }
	}
	if (!bAnyWeapon)
	{
		UE_LOG(LogQuakeInventory, Warning,
			TEXT("%s: starting loadout has no weapons — fill "
			     "BP_QuakeCharacter->InventoryComponent->OwnedWeaponClasses "
			     "(Axe at slot 1 minimum)."),
			*GetNameSafe(GetOwner()));
	}
}

// --- Armor ---

void UQuakeInventoryComponent::SetArmor(float NewArmor, float NewAbsorption)
{
	Armor = NewArmor;
	ArmorAbsorption = NewAbsorption;
}

void UQuakeInventoryComponent::ApplyArmorDamage(float NewArmor)
{
	Armor = NewArmor;
}

// --- Weapons ---

void UQuakeInventoryComponent::GiveWeapon(int32 SlotNumberOneBased, TSubclassOf<AQuakeWeaponBase> WeaponClass)
{
	const int32 Index = SlotNumberOneBased - 1;
	if (Index < 0 || Index >= OwnedWeaponClasses.Num())
	{
		return;
	}
	OwnedWeaponClasses[Index] = WeaponClass;
}

bool UQuakeInventoryComponent::OwnsWeaponInSlot(int32 SlotIndexZeroBased) const
{
	if (SlotIndexZeroBased < 0 || SlotIndexZeroBased >= OwnedWeaponClasses.Num())
	{
		return false;
	}
	return OwnedWeaponClasses[SlotIndexZeroBased] != nullptr;
}

// --- Ammo ---

int32 UQuakeInventoryComponent::GetAmmoCap(EQuakeAmmoType Type)
{
	switch (Type)
	{
	case EQuakeAmmoType::Shells:  return 100;
	case EQuakeAmmoType::Nails:   return 200;
	case EQuakeAmmoType::Rockets: return 100;
	case EQuakeAmmoType::Cells:   return 100;
	default:                      return 0;
	}
}

int32 UQuakeInventoryComponent::GetAmmo(EQuakeAmmoType Type) const
{
	if (Type == EQuakeAmmoType::None)
	{
		return 0;
	}
	const int32* Found = AmmoCounts.Find(Type);
	return Found ? *Found : 0;
}

int32 UQuakeInventoryComponent::GiveAmmo(EQuakeAmmoType Type, int32 Amount)
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

bool UQuakeInventoryComponent::ConsumeAmmo(EQuakeAmmoType Type, int32 Amount)
{
	if (Type == EQuakeAmmoType::None)
	{
		// Weapons with no ammo type (Axe) always succeed.
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

// --- Snapshot ---

void UQuakeInventoryComponent::SerializeTo(FQuakeInventorySnapshot& Out) const
{
	Out.Armor              = Armor;
	Out.ArmorAbsorption    = ArmorAbsorption;
	Out.OwnedWeaponClasses = OwnedWeaponClasses;
	Out.AmmoCounts         = AmmoCounts;
	Out.bValid             = true;
}

void UQuakeInventoryComponent::DeserializeFrom(const FQuakeInventorySnapshot& In)
{
	Armor              = In.Armor;
	ArmorAbsorption    = In.ArmorAbsorption;
	OwnedWeaponClasses = In.OwnedWeaponClasses;
	AmmoCounts         = In.AmmoCounts;

	// Pre-seed missing ammo keys so GetAmmo always finds an entry downstream.
	for (EQuakeAmmoType Type : { EQuakeAmmoType::Shells, EQuakeAmmoType::Nails,
								  EQuakeAmmoType::Rockets, EQuakeAmmoType::Cells })
	{
		AmmoCounts.FindOrAdd(Type);
	}

	// Snapshots may pre-date a NumWeaponSlots expansion; keep the array
	// full-width so slot indexing doesn't walk off the end.
	if (OwnedWeaponClasses.Num() < AQuakeCharacter::NumWeaponSlots)
	{
		OwnedWeaponClasses.SetNum(AQuakeCharacter::NumWeaponSlots);
	}
}
