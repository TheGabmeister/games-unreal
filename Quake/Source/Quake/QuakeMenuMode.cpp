#include "QuakeMenuMode.h"

#include "SQuakeMenuWidgets.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

AQuakeMenuGameMode::AQuakeMenuGameMode()
{
	// Default pawn / controller — menu doesn't need a Quake character. The
	// engine spawns a stock APawn for the controller, which is harmless.
	HUDClass = AQuakeMenuHUD::StaticClass();
}

void AQuakeMenuHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !GEngine || !GEngine->GameViewport)
	{
		return;
	}
	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	MenuWidget = SNew(SQuakeMainMenu)
		.OwningPlayerController(PC)
		.HubMapName(HubMapName);

	GEngine->GameViewport->AddViewportWidgetForPlayer(
		LocalPlayer, MenuWidget.ToSharedRef(), /*ZOrder*/ 50);

	// Show cursor + UI-only input while the menu is up.
	PC->bShowMouseCursor = true;
	PC->SetInputMode(FInputModeUIOnly());
}

void AQuakeMenuHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MenuWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		if (APlayerController* PC = GetOwningPlayerController())
		{
			if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
			{
				GEngine->GameViewport->RemoveViewportWidgetForPlayer(
					LocalPlayer, MenuWidget.ToSharedRef());
			}
		}
		MenuWidget.Reset();
	}
	Super::EndPlay(EndPlayReason);
}
