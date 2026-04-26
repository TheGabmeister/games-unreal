#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonTile.h"
#include "DiabloDungeonGenerator.generated.h"

class ADiabloEnemy;
class ADungeonStairs;
class UTilePalette;

UCLASS()
class DIABLO_API ADiabloDungeonGenerator : public AActor
{
	GENERATED_BODY()

public:
	ADiabloDungeonGenerator();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FName DungeonFloorName = TEXT("Cathedral_L1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 GridWidth = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 GridHeight = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 TargetRoomCount = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 TargetEnemyCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TObjectPtr<UTilePalette> TilePalette;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TSubclassOf<ADiabloEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TSubclassOf<AActor> HealingPotionClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TSubclassOf<ADungeonStairs> StairsClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FName ReturnLevelName = TEXT("Lvl_Diablo");

private:
	struct FGridRoom
	{
		int32 X = 0;
		int32 Y = 0;
		int32 W = 0;
		int32 H = 0;

		FIntPoint Center() const
		{
			return FIntPoint(X + W / 2, Y + H / 2);
		}
	};

	void Generate();
	void BuildLayout(FRandomStream& Stream);
	bool TryBudRoom(const FGridRoom& BaseRoom, FRandomStream& Stream);
	bool CanCarveRect(int32 X, int32 Y, int32 W, int32 H, int32 Margin) const;
	void CarveRect(int32 X, int32 Y, int32 W, int32 H);
	void AddBorderWalls();
	void SpawnTiles();
	void SpawnGameplayActors(FRandomStream& Stream);
	void RefreshNavigation();

	int32 GetIndex(int32 X, int32 Y) const;
	bool IsInBounds(int32 X, int32 Y) const;
	FVector GetTileWorldLocation(int32 X, int32 Y, float Z = 0.f) const;
	float GetTileSize() const;

	TArray<EDungeonTileType> Grid;
	TArray<FGridRoom> Rooms;
	TArray<TObjectPtr<AActor>> SpawnedActors;
	bool bGenerated = false;
};
