#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiabloMainMenu.generated.h"

class UButton;
class UTextBlock;
class ADiabloPlayerController;

UCLASS(Abstract)
class DIABLO_API UDiabloMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(ADiabloPlayerController* InController);
	void UpdateButtonStates(bool bCanSave, bool bCanLoad);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnSaveClicked();

	UFUNCTION()
	void OnLoadClicked();

	UPROPERTY()
	TObjectPtr<ADiabloPlayerController> OwnerController;

	UPROPERTY()
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY()
	TObjectPtr<UButton> SaveButton;

	UPROPERTY()
	TObjectPtr<UButton> LoadButton;

	UPROPERTY()
	TObjectPtr<UTextBlock> SaveButtonText;
};
