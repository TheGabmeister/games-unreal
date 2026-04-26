#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiabloDialogWidget.generated.h"

class UTextBlock;
class UButton;
class ADiabloPlayerController;

UCLASS(Abstract)
class DIABLO_API UDiabloDialogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(ADiabloPlayerController* InController);
	void SetDialog(const FText& Name, const FText& Text);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UFUNCTION()
	void OnCloseClicked();

	UPROPERTY()
	TObjectPtr<ADiabloPlayerController> OwnerController;

	UPROPERTY()
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY()
	TObjectPtr<UTextBlock> DialogTextBlock;

	UPROPERTY()
	TObjectPtr<UButton> CloseButton;
};
