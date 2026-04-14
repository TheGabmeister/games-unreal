#include "QuakeTrigger_Secret.h"

#include "QuakeHUD.h"
#include "QuakePlayerState.h"
#include "QuakeSaveArchive.h"

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

	UE_LOG(LogQuakeSecret, Log, TEXT("%s: secret found (instigator %s)"),
		*GetName(), *GetNameSafe(InInstigator));

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		// SPEC 5.9: credit the secret to the player's numerator.
		if (AQuakePlayerState* PS = PC->GetPlayerState<AQuakePlayerState>())
		{
			PS->AddSecretCredit();
		}
		if (AQuakeHUD* HUD = Cast<AQuakeHUD>(PC->GetHUD()))
		{
			HUD->ShowMessage(NSLOCTEXT("Quake", "SecretFound", "A secret area!"), 3.f);
		}
	}

	Super::Activate(InInstigator);
}

void AQuakeTrigger_Secret::SaveState(FActorSaveRecord& OutRecord)
{
	OutRecord.ActorName = GetFName();
	QuakeSaveArchive::WriteSaveProperties(this, OutRecord.Payload);
}

void AQuakeTrigger_Secret::LoadState(const FActorSaveRecord& InRecord)
{
	QuakeSaveArchive::ReadSaveProperties(this, InRecord.Payload);
	// bCredited controls re-credit gating in Activate — no further wiring needed.
}
