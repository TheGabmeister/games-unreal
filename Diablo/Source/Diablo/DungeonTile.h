#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonTile.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EDungeonTileType : uint8
{
	Empty,
	Floor,
	Wall
};

UCLASS()
class DIABLO_API ADungeonTile : public AActor
{
	GENERATED_BODY()

public:
	ADungeonTile();

	void SetTileType(EDungeonTileType InTileType);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	EDungeonTileType TileType = EDungeonTileType::Floor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dungeon")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
