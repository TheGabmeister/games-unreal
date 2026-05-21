#include "TH2PlayerController.h"
#include "TH2SkaterPawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

void ATH2PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (SkatingMappingContext)
		{
			Subsystem->AddMappingContext(SkatingMappingContext, 0);
		}
	}
}

void ATH2PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC) return;

	if (MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATH2PlayerController::HandleMove);
	}
	if (OllieAction)
	{
		EIC->BindAction(OllieAction, ETriggerEvent::Started, this, &ATH2PlayerController::HandleOllieStarted);
		EIC->BindAction(OllieAction, ETriggerEvent::Completed, this, &ATH2PlayerController::HandleOllieCompleted);
	}
	if (BrakeAction)
	{
		EIC->BindAction(BrakeAction, ETriggerEvent::Started, this, &ATH2PlayerController::HandleBrakeStarted);
		EIC->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ATH2PlayerController::HandleBrakeCompleted);
	}
	if (SwitchStanceAction)
	{
		EIC->BindAction(SwitchStanceAction, ETriggerEvent::Started, this, &ATH2PlayerController::HandleSwitchStance);
	}
}

void ATH2PlayerController::HandleMove(const FInputActionValue& Value)
{
	if (ATH2SkaterPawn* Skater = Cast<ATH2SkaterPawn>(GetPawn()))
	{
		Skater->SetMoveInput(Value.Get<FVector2D>());
	}
}

void ATH2PlayerController::HandleOllieStarted(const FInputActionValue& Value)
{
	if (ATH2SkaterPawn* Skater = Cast<ATH2SkaterPawn>(GetPawn()))
	{
		Skater->StartOllie();
	}
}

void ATH2PlayerController::HandleOllieCompleted(const FInputActionValue& Value)
{
	if (ATH2SkaterPawn* Skater = Cast<ATH2SkaterPawn>(GetPawn()))
	{
		Skater->ReleaseOllie();
	}
}

void ATH2PlayerController::HandleBrakeStarted(const FInputActionValue& Value)
{
	if (ATH2SkaterPawn* Skater = Cast<ATH2SkaterPawn>(GetPawn()))
	{
		Skater->SetBraking(true);
	}
}

void ATH2PlayerController::HandleBrakeCompleted(const FInputActionValue& Value)
{
	if (ATH2SkaterPawn* Skater = Cast<ATH2SkaterPawn>(GetPawn()))
	{
		Skater->SetBraking(false);
	}
}

void ATH2PlayerController::HandleSwitchStance(const FInputActionValue& Value)
{
	if (ATH2SkaterPawn* Skater = Cast<ATH2SkaterPawn>(GetPawn()))
	{
		Skater->ToggleSwitchStance();
	}
}
