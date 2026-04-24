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

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void ImportWarriorFBX();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void ImportAttackSFX();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void ConfigureBlueprintDefaults();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void ConfigureHeroDefaults();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void ConfigureControllerDefaults();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void ConfigureGameModeDefaults();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void ConfigureEnemyDefaults();
};
