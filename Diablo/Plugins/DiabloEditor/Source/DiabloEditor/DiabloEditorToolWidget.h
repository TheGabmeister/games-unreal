#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "DiabloEditorToolWidget.generated.h"

UCLASS()
class UDiabloEditorToolWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void GenerateAllAssets();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void GenerateBlueprintSubclasses();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void GenerateDefaultMap();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void GenerateInputAssets();

private:
	bool CreateBlueprintFromClass(UClass* ParentClass, const FString& AssetPath, const FString& AssetName);
	bool SaveAsset(UObject* Asset, UPackage* Package, const FString& PackagePath);
	void NotifyAssetCreated(UObject* Asset);
};
