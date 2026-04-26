#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiabloHUDWidget.generated.h"

class UProgressBar;
class USizeBox;
class UTextBlock;
class UBorder;
class UImage;
class ADiabloHero;
class UInventoryComponent;

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
	void OnInventoryChanged();
	void RefreshBars();
	void RefreshBelt();

	UPROPERTY()
	TObjectPtr<ADiabloHero> CachedHero;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> CachedInventory;

	UPROPERTY()
	TObjectPtr<UProgressBar> LifeBar;

	UPROPERTY()
	TObjectPtr<UProgressBar> ManaBar;

	UPROPERTY()
	TObjectPtr<UProgressBar> XPBar;

	UPROPERTY()
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY()
	TArray<TObjectPtr<UBorder>> BeltSlotWidgets;

	FDelegateHandle StatsChangedHandle;
	FDelegateHandle InventoryChangedHandle;
};
