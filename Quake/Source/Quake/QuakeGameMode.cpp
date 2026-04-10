#include "QuakeGameMode.h"
#include "QuakeCharacter.h"
#include "QuakePlayerController.h"

AQuakeGameMode::AQuakeGameMode()
{
	DefaultPawnClass = AQuakeCharacter::StaticClass();
	PlayerControllerClass = AQuakePlayerController::StaticClass();
}
