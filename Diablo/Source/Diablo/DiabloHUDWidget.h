#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiabloHUDWidget.generated.h"

class UProgressBar;
class USizeBox;
class UTextBlock;
class ADiabloHero;

UCLASS(Abstract)
class DIABLO_API UDiabloHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitForHero(ADiabloHero* InHero);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void OnStatsChanged();
	void RefreshBars();

	UPROPERTY()
	TObjectPtr<ADiabloHero> CachedHero;

	UPROPERTY()
	TObjectPtr<UProgressBar> LifeBar;

	UPROPERTY()
	TObjectPtr<UProgressBar> ManaBar;

	UPROPERTY()
	TObjectPtr<UProgressBar> XPBar;

	UPROPERTY()
	TObjectPtr<UTextBlock> LevelText;

	FDelegateHandle StatsChangedHandle;
};
