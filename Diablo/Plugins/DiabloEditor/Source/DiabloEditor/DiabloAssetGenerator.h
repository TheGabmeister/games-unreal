#pragma once

#include "CoreMinimal.h"

class UInputAction;

struct FDiabloAssetGenerator
{
	static void GenerateAllAssets();
	static void GenerateBlueprintSubclasses();
	static void GenerateDefaultMap();
	static void GenerateInputAssets();

private:
	static bool CreateBlueprintFromClass(UClass* ParentClass, const FString& AssetPath, const FString& AssetName);
	static bool SaveAsset(UObject* Asset, UPackage* Package, const FString& PackagePath);
	static void NotifyAssetCreated(UObject* Asset);
};
