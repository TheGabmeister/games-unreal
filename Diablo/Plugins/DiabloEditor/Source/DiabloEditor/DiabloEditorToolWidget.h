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
	void GenerateDefaultMap();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void GenerateInputAssets();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void ImportWarriorFBX();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void ImportAttackSFX();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void ImportLevelUpSFX();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void ImportPotionSprite();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void SetupAllBlueprints();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void SetupHero();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void SetupController();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void SetupGameMode();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void SetupEnemy();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void SetupPotion();

	UFUNCTION(BlueprintCallable, Category = "Diablo Tools")
	void SetupHUD();
};
