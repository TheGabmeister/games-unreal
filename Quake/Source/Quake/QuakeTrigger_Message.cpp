#include "QuakeTrigger_Message.h"

#include "QuakeHUD.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void AQuakeTrigger_Message::Activate(AActor* InInstigator)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (AQuakeHUD* HUD = Cast<AQuakeHUD>(PC->GetHUD()))
		{
			HUD->ShowMessage(Message, Duration);
		}
	}

	Super::Activate(InInstigator);
}
