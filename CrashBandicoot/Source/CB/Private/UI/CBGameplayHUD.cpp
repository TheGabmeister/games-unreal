#include "UI/CBGameplayHUD.h"
#include "Components/TextBlock.h"

void UCBGameplayHUD::UpdateLives(int32 Lives)
{
    if (LivesText)
    {
        LivesText->SetText(FText::AsNumber(Lives));
    }
}

void UCBGameplayHUD::UpdateWumpa(int32 Count)
{
    if (WumpaText)
    {
        WumpaText->SetText(FText::AsNumber(Count));
    }
}
