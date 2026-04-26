#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDefinition.h"
#include "InventoryDragDrop.generated.h"

UENUM()
enum class EInventoryDragSource : uint8
{
	Grid,
	Equipment,
	Belt
};

UCLASS()
class DIABLO_API UInventoryDragDrop : public UDragDropOperation
{
	GENERATED_BODY()

public:
	EInventoryDragSource Source = EInventoryDragSource::Grid;

	int32 SourceGridX = -1;
	int32 SourceGridY = -1;

	EEquipSlot SourceEquipSlot = EEquipSlot::None;

	int32 SourceBeltSlot = -1;
};
