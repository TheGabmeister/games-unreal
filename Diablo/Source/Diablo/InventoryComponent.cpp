#include "InventoryComponent.h"
#include "DiabloHero.h"
#include "Diablo.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	GridItems.SetNum(GridWidth * GridHeight);
	OccupancyGrid.SetNum(GridWidth * GridHeight);
	FMemory::Memzero(OccupancyGrid.GetData(), OccupancyGrid.Num() * sizeof(int32));
	BeltItems.SetNum(BeltSlots);
}

bool UInventoryComponent::CanPlaceAt(const UItemDefinition* Def, int32 GridX, int32 GridY, int32 IgnoreX, int32 IgnoreY) const
{
	if (!Def) return false;

	if (GridX < 0 || GridY < 0 ||
		GridX + Def->GridWidth > GridWidth ||
		GridY + Def->GridHeight > GridHeight)
	{
		return false;
	}

	for (int32 dy = 0; dy < Def->GridHeight; ++dy)
	{
		for (int32 dx = 0; dx < Def->GridWidth; ++dx)
		{
			const int32 Idx = GridIndex(GridX + dx, GridY + dy);
			if (OccupancyGrid[Idx] != 0)
			{
				if (IgnoreX >= 0 && IgnoreY >= 0)
				{
					const FItemInstance& Existing = GridItems[GridIndex(IgnoreX, IgnoreY)];
					if (Existing.IsValid() && OccupancyGrid[Idx] == GridIndex(IgnoreX, IgnoreY) + 1)
					{
						continue;
					}
				}
				return false;
			}
		}
	}

	return true;
}

bool UInventoryComponent::FindFreeSlot(const UItemDefinition* Def, int32& OutX, int32& OutY) const
{
	if (!Def) return false;

	for (int32 y = 0; y <= GridHeight - Def->GridHeight; ++y)
	{
		for (int32 x = 0; x <= GridWidth - Def->GridWidth; ++x)
		{
			if (CanPlaceAt(Def, x, y))
			{
				OutX = x;
				OutY = y;
				return true;
			}
		}
	}

	return false;
}

bool UInventoryComponent::TryAddItem(const FItemInstance& Item)
{
	if (!Item.IsValid()) return false;

	if (Item.Definition->bStackable)
	{
		for (int32 i = 0; i < GridItems.Num(); ++i)
		{
			FItemInstance& Slot = GridItems[i];
			if (Slot.IsValid() && Slot.Definition == Item.Definition &&
				Slot.StackCount < Slot.Definition->MaxStack)
			{
				int32 CanAdd = Slot.Definition->MaxStack - Slot.StackCount;
				int32 ToAdd = FMath::Min(CanAdd, Item.StackCount);
				Slot.StackCount += ToAdd;
				OnInventoryChanged.Broadcast();
				UE_LOG(LogDiablo, Display, TEXT("Stacked %d %s (now %d)"),
					ToAdd, *Item.Definition->DisplayName.ToString(), Slot.StackCount);
				return true;
			}
		}
	}

	int32 X, Y;
	if (!FindFreeSlot(Item.Definition, X, Y))
	{
		UE_LOG(LogDiablo, Warning, TEXT("Inventory full — cannot add %s"),
			*Item.Definition->DisplayName.ToString());
		return false;
	}

	return TryAddItemAt(Item, X, Y);
}

bool UInventoryComponent::TryAddItemAt(const FItemInstance& Item, int32 GridX, int32 GridY)
{
	if (!Item.IsValid()) return false;
	if (!CanPlaceAt(Item.Definition, GridX, GridY)) return false;

	const int32 Idx = GridIndex(GridX, GridY);
	GridItems[Idx] = Item;

	const int32 Marker = Idx + 1;
	for (int32 dy = 0; dy < Item.Definition->GridHeight; ++dy)
	{
		for (int32 dx = 0; dx < Item.Definition->GridWidth; ++dx)
		{
			OccupancyGrid[GridIndex(GridX + dx, GridY + dy)] = Marker;
		}
	}

	OnInventoryChanged.Broadcast();
	UE_LOG(LogDiablo, Display, TEXT("Added %s at grid (%d,%d)"),
		*Item.Definition->DisplayName.ToString(), GridX, GridY);
	return true;
}

bool UInventoryComponent::MoveItem(int32 FromX, int32 FromY, int32 ToX, int32 ToY)
{
	const int32 FromIdx = GridIndex(FromX, FromY);
	if (FromIdx < 0 || FromIdx >= GridItems.Num()) return false;

	FItemInstance& Item = GridItems[FromIdx];
	if (!Item.IsValid()) return false;

	if (!CanPlaceAt(Item.Definition, ToX, ToY, FromX, FromY)) return false;

	const int32 OldMarker = FromIdx + 1;
	for (int32 i = 0; i < OccupancyGrid.Num(); ++i)
	{
		if (OccupancyGrid[i] == OldMarker)
		{
			OccupancyGrid[i] = 0;
		}
	}

	const int32 ToIdx = GridIndex(ToX, ToY);
	GridItems[ToIdx] = Item;
	GridItems[FromIdx] = FItemInstance();

	const int32 NewMarker = ToIdx + 1;
	for (int32 dy = 0; dy < GridItems[ToIdx].Definition->GridHeight; ++dy)
	{
		for (int32 dx = 0; dx < GridItems[ToIdx].Definition->GridWidth; ++dx)
		{
			OccupancyGrid[GridIndex(ToX + dx, ToY + dy)] = NewMarker;
		}
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveItemAt(int32 GridX, int32 GridY)
{
	const int32 Idx = GridIndex(GridX, GridY);
	if (Idx < 0 || Idx >= GridItems.Num()) return false;

	FItemInstance& Item = GridItems[Idx];
	if (!Item.IsValid()) return false;

	const int32 Marker = Idx + 1;
	for (int32 i = 0; i < OccupancyGrid.Num(); ++i)
	{
		if (OccupancyGrid[i] == Marker)
		{
			OccupancyGrid[i] = 0;
		}
	}

	Item = FItemInstance();
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::UseItem(int32 GridX, int32 GridY)
{
	const int32 Idx = GridIndex(GridX, GridY);
	if (Idx < 0 || Idx >= GridItems.Num()) return false;

	FItemInstance& Item = GridItems[Idx];
	if (!Item.IsValid()) return false;

	const UItemDefinition* Def = Item.Definition;

	if (Def->UseEffect != EItemUseEffect::None)
	{
		ADiabloHero* Hero = Cast<ADiabloHero>(GetOwner());
		if (!Hero) return false;

		switch (Def->UseEffect)
		{
		case EItemUseEffect::RestoreHP:
			Hero->Heal(Def->HealAmount);
			break;
		case EItemUseEffect::RestoreMana:
			Hero->RestoreMana(Def->ManaRestoreAmount);
			break;
		case EItemUseEffect::RestoreBoth:
			Hero->Heal(Def->HealAmount);
			Hero->RestoreMana(Def->ManaRestoreAmount);
			break;
		case EItemUseEffect::CastSpell:
			if (Def->ScrollSpell)
			{
				if (!Hero->CastSpellFromScroll(Def->ScrollSpell))
				{
					return false;
				}
			}
			break;
		default:
			break;
		}

		UE_LOG(LogDiablo, Display, TEXT("Used %s"), *Def->DisplayName.ToString());

		if (Item.StackCount > 1)
		{
			Item.StackCount--;
		}
		else
		{
			RemoveItemAt(GridX, GridY);
		}

		OnInventoryChanged.Broadcast();
		return true;
	}

	if (Def->EquipSlot != EEquipSlot::None)
	{
		return Equip(GridX, GridY);
	}

	return false;
}

bool UInventoryComponent::Equip(int32 GridX, int32 GridY)
{
	const int32 Idx = GridIndex(GridX, GridY);
	if (Idx < 0 || Idx >= GridItems.Num()) return false;

	FItemInstance& Item = GridItems[Idx];
	if (!Item.IsValid()) return false;

	const EEquipSlot Slot = Item.Definition->EquipSlot;
	if (Slot == EEquipSlot::None) return false;

	FItemInstance PreviouslyEquipped;
	if (EquippedItems.Contains(Slot))
	{
		PreviouslyEquipped = EquippedItems[Slot];
	}

	FItemInstance ToEquip = Item;
	RemoveItemAt(GridX, GridY);

	EquippedItems.Add(Slot, ToEquip);

	if (PreviouslyEquipped.IsValid())
	{
		TryAddItem(PreviouslyEquipped);
	}

	if (ADiabloHero* Hero = Cast<ADiabloHero>(GetOwner()))
	{
		Hero->RecomputeDerivedStats();
		Hero->OnStatsChanged.Broadcast();
	}

	OnInventoryChanged.Broadcast();
	UE_LOG(LogDiablo, Display, TEXT("Equipped %s in slot %d"),
		*ToEquip.Definition->DisplayName.ToString(), static_cast<int32>(Slot));
	return true;
}

bool UInventoryComponent::Unequip(EEquipSlot Slot)
{
	if (!EquippedItems.Contains(Slot)) return false;

	FItemInstance Item = EquippedItems[Slot];
	if (!Item.IsValid()) return false;

	if (!TryAddItem(Item))
	{
		UE_LOG(LogDiablo, Warning, TEXT("Cannot unequip %s — inventory full"),
			*Item.Definition->DisplayName.ToString());
		return false;
	}

	EquippedItems.Remove(Slot);

	if (ADiabloHero* Hero = Cast<ADiabloHero>(GetOwner()))
	{
		Hero->RecomputeDerivedStats();
		Hero->OnStatsChanged.Broadcast();
	}

	OnInventoryChanged.Broadcast();
	UE_LOG(LogDiablo, Display, TEXT("Unequipped %s from slot %d"),
		*Item.Definition->DisplayName.ToString(), static_cast<int32>(Slot));
	return true;
}

const FItemInstance* UInventoryComponent::GetItemAt(int32 GridX, int32 GridY) const
{
	const int32 Idx = GridIndex(GridX, GridY);
	if (Idx < 0 || Idx >= GridItems.Num()) return nullptr;
	if (!GridItems[Idx].IsValid()) return nullptr;
	return &GridItems[Idx];
}

const FItemInstance& UInventoryComponent::GetEquipped(EEquipSlot Slot) const
{
	static const FItemInstance Empty;
	const FItemInstance* Found = EquippedItems.Find(Slot);
	return Found ? *Found : Empty;
}

bool UInventoryComponent::HasEquipped(EEquipSlot Slot) const
{
	const FItemInstance* Found = EquippedItems.Find(Slot);
	return Found && Found->IsValid();
}

void UInventoryComponent::AddGold(int32 Amount)
{
	if (Amount <= 0) return;
	Gold += Amount;
	OnInventoryChanged.Broadcast();
	UE_LOG(LogDiablo, Display, TEXT("Gained %d gold (total: %d)"), Amount, Gold);
}

bool UInventoryComponent::SpendGold(int32 Amount)
{
	if (Amount <= 0 || Gold < Amount) return false;
	Gold -= Amount;
	OnInventoryChanged.Broadcast();
	return true;
}

int32 UInventoryComponent::IdentifyAll()
{
	const int32 CostPerItem = 100;
	int32 Count = 0;

	for (FItemInstance& Item : GridItems)
	{
		if (!Item.IsValid() || Item.bIdentified || Item.Affixes.Num() == 0)
		{
			continue;
		}
		if (Gold < CostPerItem)
		{
			break;
		}
		Gold -= CostPerItem;
		Item.bIdentified = true;
		++Count;
	}

	for (auto& Pair : EquippedItems)
	{
		FItemInstance& Item = Pair.Value;
		if (!Item.IsValid() || Item.bIdentified || Item.Affixes.Num() == 0)
		{
			continue;
		}
		if (Gold < CostPerItem)
		{
			break;
		}
		Gold -= CostPerItem;
		Item.bIdentified = true;
		++Count;
	}

	if (Count > 0)
	{
		OnInventoryChanged.Broadcast();
	}

	return Count;
}

bool UInventoryComponent::IsBeltCompatible(const UItemDefinition* Def)
{
	if (!Def) return false;
	return Def->Category == EItemCategory::Potion || Def->Category == EItemCategory::Scroll;
}

bool UInventoryComponent::TryAddToBelt(const FItemInstance& Item)
{
	if (!Item.IsValid() || !IsBeltCompatible(Item.Definition)) return false;

	for (int32 i = 0; i < BeltSlots; ++i)
	{
		if (Item.Definition->bStackable && BeltItems[i].IsValid() &&
			BeltItems[i].Definition == Item.Definition &&
			BeltItems[i].StackCount < BeltItems[i].Definition->MaxStack)
		{
			int32 CanAdd = BeltItems[i].Definition->MaxStack - BeltItems[i].StackCount;
			int32 ToAdd = FMath::Min(CanAdd, Item.StackCount);
			BeltItems[i].StackCount += ToAdd;
			OnInventoryChanged.Broadcast();
			return true;
		}
	}

	for (int32 i = 0; i < BeltSlots; ++i)
	{
		if (!BeltItems[i].IsValid())
		{
			BeltItems[i] = Item;
			OnInventoryChanged.Broadcast();
			UE_LOG(LogDiablo, Display, TEXT("Added %s to belt slot %d"),
				*Item.Definition->DisplayName.ToString(), i + 1);
			return true;
		}
	}

	return false;
}

bool UInventoryComponent::SetBeltItem(int32 Slot, const FItemInstance& Item)
{
	if (Slot < 0 || Slot >= BeltSlots) return false;
	if (Item.IsValid() && !IsBeltCompatible(Item.Definition)) return false;

	BeltItems[Slot] = Item;
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::UseBeltSlot(int32 Slot)
{
	if (Slot < 0 || Slot >= BeltSlots) return false;

	FItemInstance& Item = BeltItems[Slot];
	if (!Item.IsValid()) return false;

	const UItemDefinition* Def = Item.Definition;
	ADiabloHero* Hero = Cast<ADiabloHero>(GetOwner());
	if (!Hero) return false;

	switch (Def->UseEffect)
	{
	case EItemUseEffect::RestoreHP:
		Hero->Heal(Def->HealAmount);
		break;
	case EItemUseEffect::RestoreMana:
		Hero->RestoreMana(Def->ManaRestoreAmount);
		break;
	case EItemUseEffect::RestoreBoth:
		Hero->Heal(Def->HealAmount);
		Hero->RestoreMana(Def->ManaRestoreAmount);
		break;
	case EItemUseEffect::CastSpell:
		if (Def->ScrollSpell)
		{
			if (!Hero->CastSpellFromScroll(Def->ScrollSpell))
			{
				return false;
			}
		}
		break;
	default:
		return false;
	}

	UE_LOG(LogDiablo, Display, TEXT("Used belt slot %d: %s"), Slot + 1, *Def->DisplayName.ToString());

	if (Item.StackCount > 1)
	{
		Item.StackCount--;
	}
	else
	{
		Item = FItemInstance();
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveBeltSlot(int32 Slot)
{
	if (Slot < 0 || Slot >= BeltSlots) return false;
	if (!BeltItems[Slot].IsValid()) return false;

	BeltItems[Slot] = FItemInstance();
	OnInventoryChanged.Broadcast();
	return true;
}

const FItemInstance& UInventoryComponent::GetBeltItem(int32 Slot) const
{
	static const FItemInstance Empty;
	if (Slot < 0 || Slot >= BeltSlots) return Empty;
	return BeltItems[Slot];
}

bool UInventoryComponent::MoveGridToBelt(int32 GridX, int32 GridY)
{
	const int32 Idx = GridIndex(GridX, GridY);
	if (Idx < 0 || Idx >= GridItems.Num()) return false;

	FItemInstance& Item = GridItems[Idx];
	if (!Item.IsValid() || !IsBeltCompatible(Item.Definition)) return false;

	if (TryAddToBelt(Item))
	{
		RemoveItemAt(GridX, GridY);
		return true;
	}

	return false;
}

void UInventoryComponent::RestoreState(const TArray<FItemInstance>& InGridItems, const TArray<int32>& InOccupancy,
	const TMap<EEquipSlot, FItemInstance>& InEquipped, int32 InGold,
	const TArray<FItemInstance>& InBeltItems)
{
	GridItems = InGridItems;
	OccupancyGrid = InOccupancy;
	EquippedItems = InEquipped;
	Gold = InGold;
	BeltItems = InBeltItems;
	if (BeltItems.Num() < BeltSlots)
	{
		BeltItems.SetNum(BeltSlots);
	}
	OnInventoryChanged.Broadcast();
}
