#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiabloCharacterPanel.generated.h"

class UTextBlock;
class UButton;
class ADiabloHero;

UCLASS(Abstract)
class DIABLO_API UDiabloCharacterPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitForHero(ADiabloHero* InHero);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void OnStatsChanged();
	void RefreshDisplay();

	UFUNCTION()
	void OnStrPlus();

	UFUNCTION()
	void OnMagPlus();

	UFUNCTION()
	void OnDexPlus();

	UFUNCTION()
	void OnVitPlus();

	UPROPERTY()
	TObjectPtr<ADiabloHero> CachedHero;

	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY()
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY()
	TObjectPtr<UTextBlock> PointsText;

	UPROPERTY()
	TObjectPtr<UTextBlock> StrText;

	UPROPERTY()
	TObjectPtr<UTextBlock> MagText;

	UPROPERTY()
	TObjectPtr<UTextBlock> DexText;

	UPROPERTY()
	TObjectPtr<UTextBlock> VitText;

	UPROPERTY()
	TObjectPtr<UTextBlock> HPText;

	UPROPERTY()
	TObjectPtr<UTextBlock> ManaText;

	UPROPERTY()
	TObjectPtr<UButton> StrButton;

	UPROPERTY()
	TObjectPtr<UButton> MagButton;

	UPROPERTY()
	TObjectPtr<UButton> DexButton;

	UPROPERTY()
	TObjectPtr<UButton> VitButton;

	FDelegateHandle StatsChangedHandle;
};
