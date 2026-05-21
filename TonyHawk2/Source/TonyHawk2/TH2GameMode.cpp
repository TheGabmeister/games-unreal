#include "TH2GameMode.h"
#include "TH2SkaterPawn.h"
#include "TH2PlayerController.h"
#include "UObject/ConstructorHelpers.h"

ATH2GameMode::ATH2GameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PawnBP(TEXT("/Game/Phase1/Blueprints/BP_SkaterPawn"));
	if (PawnBP.Succeeded())
		DefaultPawnClass = PawnBP.Class;
	else
		DefaultPawnClass = ATH2SkaterPawn::StaticClass();

	static ConstructorHelpers::FClassFinder<APlayerController> ControllerBP(TEXT("/Game/Phase1/Blueprints/BP_PlayerController"));
	if (ControllerBP.Succeeded())
		PlayerControllerClass = ControllerBP.Class;
	else
		PlayerControllerClass = ATH2PlayerController::StaticClass();
}
