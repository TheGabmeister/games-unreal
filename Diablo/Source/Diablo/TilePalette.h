#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DungeonTile.h"
#include "TilePalette.generated.h"

USTRUCT(BlueprintType)
struct FTilePaletteEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tile")
	EDungeonTileType TileType = EDungeonTileType::Floor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tile")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tile")
	FVector Scale = FVector(1.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tile")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tile")
	TObjectPtr<UMaterialInterface> Material;
};

UCLASS(BlueprintType)
class DIABLO_API UTilePalette : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon")
	float TileSize = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dungeon")
	TArray<FTilePaletteEntry> Entries;

	const FTilePaletteEntry* FindEntry(EDungeonTileType TileType) const;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("TilePalette", GetFName());
	}
};
