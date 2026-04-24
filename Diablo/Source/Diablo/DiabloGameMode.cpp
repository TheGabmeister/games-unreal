#include "DiabloGameMode.h"
#include "DiabloHero.h"
#include "DiabloPlayerController.h"

ADiabloGameMode::ADiabloGameMode()
{
	DefaultPawnClass = ADiabloHero::StaticClass();
	PlayerControllerClass = ADiabloPlayerController::StaticClass();
}
