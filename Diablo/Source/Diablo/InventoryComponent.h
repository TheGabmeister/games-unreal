#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemInstance.h"
#include "InventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DIABLO_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	static constexpr int32 GridWidth = 10;
	static constexpr int32 GridHeight = 4;
	static constexpr int32 NumEquipSlots = 7;

	bool TryAddItem(const FItemInstance& Item);
	bool TryAddItemAt(const FItemInstance& Item, int32 GridX, int32 GridY);
	bool MoveItem(int32 FromX, int32 FromY, int32 ToX, int32 ToY);
	bool RemoveItemAt(int32 GridX, int32 GridY);

	bool Equip(int32 GridX, int32 GridY);
	bool Unequip(EEquipSlot Slot);
	bool UseItem(int32 GridX, int32 GridY);

	const FItemInstance* GetItemAt(int32 GridX, int32 GridY) const;
	const FItemInstance& GetEquipped(EEquipSlot Slot) const;
	bool HasEquipped(EEquipSlot Slot) const;

	int32 GetGold() const { return Gold; }
	void AddGold(int32 Amount);
	bool SpendGold(int32 Amount);

	FOnInventoryChanged OnInventoryChanged;

private:
	bool CanPlaceAt(const UItemDefinition* Def, int32 GridX, int32 GridY, int32 IgnoreX = -1, int32 IgnoreY = -1) const;
	bool FindFreeSlot(const UItemDefinition* Def, int32& OutX, int32& OutY) const;
	int32 GridIndex(int32 X, int32 Y) const { return Y * GridWidth + X; }

	UPROPERTY()
	TArray<FItemInstance> GridItems;

	UPROPERTY()
	TArray<int32> OccupancyGrid;

	UPROPERTY()
	TMap<EEquipSlot, FItemInstance> EquippedItems;

	UPROPERTY()
	int32 Gold = 0;
};
