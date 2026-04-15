// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF7PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

void AFF7PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (UInputMappingContext* IMC = DefaultMappingContext.LoadSynchronous())
			{
				Subsystem->AddMappingContext(IMC, 0);
			}
		}
	}
}

void AFF7PlayerController::OnUnPossess()
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (UInputMappingContext* IMC = DefaultMappingContext.Get())
			{
				Subsystem->RemoveMappingContext(IMC);
			}
		}
	}

	Super::OnUnPossess();
}

void AFF7PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC)
	{
		return;
	}

	if (UInputAction* MoveAction = IA_Move.LoadSynchronous())
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFF7PlayerController::HandleMove);
	}
	// IA_Interact / IA_MenuToggle / IA_Escape bindings land in later phases.
}

void AFF7PlayerController::HandleMove(const FInputActionValue& Value)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	const FVector2D Axis = Value.Get<FVector2D>();
	// Top-down: world X = forward, world Y = right. Input is framed the same way.
	ControlledPawn->AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Axis.X);
	ControlledPawn->AddMovementInput(FVector(0.0f, 1.0f, 0.0f), Axis.Y);
}
