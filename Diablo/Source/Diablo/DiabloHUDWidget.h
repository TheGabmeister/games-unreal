#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiabloHUDWidget.generated.h"

class UOverlay;
class UImage;
class UProgressBar;
class USizeBox;
class ADiabloHero;

UCLASS(Abstract)
class DIABLO_API UDiabloHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitForHero(ADiabloHero* InHero);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void BuildWidgetTree();

	UPROPERTY()
	TObjectPtr<ADiabloHero> CachedHero;

	UPROPERTY()
	TObjectPtr<UProgressBar> LifeBar;

	UPROPERTY()
	TObjectPtr<UProgressBar> ManaBar;

	UPROPERTY()
	TObjectPtr<UProgressBar> XPBar;
};
