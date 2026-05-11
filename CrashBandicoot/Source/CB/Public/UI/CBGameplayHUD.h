#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CBGameplayHUD.generated.h"

class UTextBlock;

UCLASS()
class CB_API UCBGameplayHUD : public UUserWidget
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "CB")
    void UpdateLives(int32 Lives);

    UFUNCTION(BlueprintCallable, Category = "CB")
    void UpdateWumpa(int32 Count);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> LivesText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> WumpaText;
};
