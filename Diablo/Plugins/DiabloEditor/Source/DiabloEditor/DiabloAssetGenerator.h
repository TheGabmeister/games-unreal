#pragma once

#include "CoreMinimal.h"

class UInputAction;
class UBlueprint;

struct FDiabloAssetGenerator
{
	static void GenerateAllAssets();
	static void GenerateBlueprintSubclasses();
	static void GenerateDefaultMap();
	static void GenerateInputAssets();
	static void ImportWarriorFBX();
	static void ImportAttackSFX();
	static void ConfigureBlueprintDefaults();

private:
	static void SpawnFloorPlane(UWorld* World);
	static void SpawnNavMeshVolume(UWorld* World);
	static bool CreateBlueprintFromClass(UClass* ParentClass, const FString& AssetPath, const FString& AssetName);
	static bool SaveAsset(UObject* Asset, UPackage* Package, const FString& PackagePath);
	static void NotifyAssetCreated(UObject* Asset);
};
