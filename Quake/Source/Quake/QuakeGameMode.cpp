#include "QuakeGameMode.h"
#include "QuakeCharacter.h"
#include "QuakeHUD.h"
#include "QuakePlayerController.h"
#include "QuakePlayerState.h"

AQuakeGameMode::AQuakeGameMode()
{
	DefaultPawnClass = AQuakeCharacter::StaticClass();
	PlayerControllerClass = AQuakePlayerController::StaticClass();
	PlayerStateClass = AQuakePlayerState::StaticClass();
	HUDClass = AQuakeHUD::StaticClass();
}
