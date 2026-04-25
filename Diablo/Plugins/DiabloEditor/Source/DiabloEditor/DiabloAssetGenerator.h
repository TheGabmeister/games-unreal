#pragma once

#include "CoreMinimal.h"

class UInputAction;
class UBlueprint;

struct FDiabloAssetGenerator
{
	static void GenerateAllAssets();
	static void GenerateDefaultMap();
	static void GenerateInputAssets();
	static void ImportWarriorFBX();
	static void ImportAttackSFX();
	static void ImportLevelUpSFX();
	static void ImportPotionSprite();
	static void ImportItemIcons();

	static void SetupHero();
	static void SetupController();
	static void SetupGameMode();
	static void SetupEnemy();
	static void SetupPotion();
	static void SetupHUD();
	static void SetupInventory();
	static void SetupDropMaterial();
	static void SetupSpells();
	static void SetupAllBlueprints();

private:
	static void SpawnFloorPlane(UWorld* World);
	static void SpawnNavMeshVolume(UWorld* World);
	static bool CreateBlueprintFromClass(UClass* ParentClass, const FString& AssetPath, const FString& AssetName);
	static bool SaveAsset(UObject* Asset, UPackage* Package, const FString& PackagePath);
	static void NotifyAssetCreated(UObject* Asset);
};
