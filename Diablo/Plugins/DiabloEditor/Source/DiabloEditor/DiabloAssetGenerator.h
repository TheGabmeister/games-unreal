#pragma once

#include "CoreMinimal.h"

class UInputAction;
class UAnimBlueprint;
class UAnimSequence;
class USkeletalMesh;
class USkeleton;
class UBlueprint;
class UAnimStateNode;
class UAnimationStateMachineGraph;
class UEdGraph;

struct FDiabloAssetGenerator
{
	static void GenerateAllAssets();
	static void GenerateBlueprintSubclasses();
	static void GenerateDefaultMap();
	static void GenerateInputAssets();
	static void ImportWarriorFBX();
	static void GenerateAnimBlueprint();
	static void ConfigureBlueprintDefaults();

private:
	static void SpawnFloorPlane(UWorld* World);
	static void SpawnNavMeshVolume(UWorld* World);
	static void DeleteExistingAsset(const FString& PackagePath);
	static bool CreateBlueprintFromClass(UClass* ParentClass, const FString& AssetPath, const FString& AssetName);
	static bool SaveAsset(UObject* Asset, UPackage* Package, const FString& PackagePath);
	static void NotifyAssetCreated(UObject* Asset);

	static UAnimStateNode* SpawnStateNode(UAnimationStateMachineGraph* Graph, const FString& Name, UAnimSequence* Anim, const FVector2f& Pos);
	static void CreateTransitionRule(UAnimationStateMachineGraph* Graph, UAnimStateNode* From, UAnimStateNode* To, bool bGreaterThan, float Threshold, const FVector2f& Pos);
};
