#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiabloSpellbookPanel.generated.h"

class UTextBlock;
class UButton;
class UVerticalBox;
class ADiabloHero;
class USpellDefinition;

UCLASS(Abstract)
class DIABLO_API UDiabloSpellbookPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitForHero(ADiabloHero* InHero);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void RefreshDisplay();
	int32 GetSpellIndexAtPosition(const FGeometry& InGeometry, const FVector2D& ScreenPosition) const;

	UPROPERTY()
	TObjectPtr<ADiabloHero> CachedHero;

	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY()
	TObjectPtr<UTextBlock> ActiveSpellText;

	UPROPERTY()
	TObjectPtr<UVerticalBox> SpellListBox;

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> SpellLabels;

	static constexpr float RowHeight = 28.f;
	static constexpr float HeaderHeight = 60.f;
};
