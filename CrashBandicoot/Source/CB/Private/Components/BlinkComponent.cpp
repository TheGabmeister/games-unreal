#include "Components/BlinkComponent.h"
#include "Components/MeshComponent.h"
#include "TimerManager.h"

UBlinkComponent::UBlinkComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UBlinkComponent::StartBlinking()
{
    if (bIsBlinking) return;
    bIsBlinking = true;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle_Blink, this, &UBlinkComponent::ToggleVisibility, BlinkInterval, true);
}

void UBlinkComponent::StopBlinking()
{
    if (!bIsBlinking) return;
    bIsBlinking = false;
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Blink);

    if (UMeshComponent* Mesh = GetOwner()->FindComponentByClass<UMeshComponent>())
    {
        Mesh->SetVisibility(true);
    }
}

void UBlinkComponent::ToggleVisibility()
{
    if (UMeshComponent* Mesh = GetOwner()->FindComponentByClass<UMeshComponent>())
    {
        Mesh->ToggleVisibility();
    }
}
