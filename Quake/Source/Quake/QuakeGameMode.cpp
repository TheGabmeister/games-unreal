#include "QuakeGameMode.h"
#include "QuakeCharacter.h"

AQuakeGameMode::AQuakeGameMode()
{
	DefaultPawnClass = AQuakeCharacter::StaticClass();
}
