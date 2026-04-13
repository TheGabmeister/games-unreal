#include "QuakeTrigger_Secret.h"

#include "QuakeHUD.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeSecret, Log, All);

void AQuakeTrigger_Secret::Activate(AActor* InInstigator)
{
	if (bCredited)
	{
		return;
	}
	bCredited = true;

	// TODO Phase 9: increment PlayerState->SecretsFound. For now, log and
	// surface the HUD message so the behavior is demonstrable.
	UE_LOG(LogQuakeSecret, Log, TEXT("%s: secret found (instigator %s)"),
		*GetName(), *GetNameSafe(InInstigator));

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (AQuakeHUD* HUD = Cast<AQuakeHUD>(PC->GetHUD()))
		{
			HUD->ShowMessage(NSLOCTEXT("Quake", "SecretFound", "A secret area!"), 3.f);
		}
	}

	Super::Activate(InInstigator);
}
