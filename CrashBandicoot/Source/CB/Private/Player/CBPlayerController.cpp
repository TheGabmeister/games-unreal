#include "Player/CBPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Player/CBPlayerCharacter.h"
#include "UI/CBGameplayHUD.h"
#include "Game/CBGameInstance.h"
#include "Blueprint/UserWidget.h"

ACBPlayerController::ACBPlayerController()
{
}

void ACBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Add mapping context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (IMC_Gameplay)
		{
			Subsystem->AddMappingContext(IMC_Gameplay, 0);
		}
	}

	// Create HUD widget
	if (GameplayHUDClass && IsLocalController())
	{
		GameplayHUD = CreateWidget<UCBGameplayHUD>(this, GameplayHUDClass);
		if (GameplayHUD)
		{
			GameplayHUD->AddToViewport();
		}
	}

	BindToGameInstance();
}

void ACBPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ACBPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
}

void ACBPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput) return;

	if (IA_MoveAxis)
	{
		EnhancedInput->BindAction(IA_MoveAxis, ETriggerEvent::Triggered, this, &ACBPlayerController::Input_Move);
		EnhancedInput->BindAction(IA_MoveAxis, ETriggerEvent::Completed, this, &ACBPlayerController::Input_Move);
	}

	if (IA_Jump)
	{
		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ACBPlayerController::Input_Jump);
		EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ACBPlayerController::Input_StopJump);
	}

	if (IA_Spin)
	{
		EnhancedInput->BindAction(IA_Spin, ETriggerEvent::Started, this, &ACBPlayerController::Input_Spin);
	}

	if (IA_PauseMenu)
	{
		EnhancedInput->BindAction(IA_PauseMenu, ETriggerEvent::Started, this, &ACBPlayerController::Input_PauseMenu);
	}
}

// --- Input forwarding to pawn ---

void ACBPlayerController::Input_Move(const FInputActionValue& Value)
{
	if (ACBPlayerCharacter* Char = GetPawn<ACBPlayerCharacter>())
	{
		Char->Input_Move(Value);
	}
}

void ACBPlayerController::Input_Jump(const FInputActionValue& Value)
{
	if (ACBPlayerCharacter* Char = GetPawn<ACBPlayerCharacter>())
	{
		Char->Input_Jump(Value);
	}
}

void ACBPlayerController::Input_StopJump(const FInputActionValue& Value)
{
	if (ACBPlayerCharacter* Char = GetPawn<ACBPlayerCharacter>())
	{
		Char->Input_StopJump(Value);
	}
}

void ACBPlayerController::Input_Spin(const FInputActionValue& Value)
{
	if (ACBPlayerCharacter* Char = GetPawn<ACBPlayerCharacter>())
	{
		Char->Input_Spin(Value);
	}
}

void ACBPlayerController::Input_PauseMenu(const FInputActionValue& Value)
{
}

// --- Bindings ---

void ACBPlayerController::BindToGameInstance()
{
	if (UCBGameInstance* GI = Cast<UCBGameInstance>(GetGameInstance()))
	{
		GI->OnLivesChanged.AddDynamic(this, &ACBPlayerController::OnLivesChanged);
		GI->OnWumpaChanged.AddDynamic(this, &ACBPlayerController::OnWumpaChanged);

		if (GameplayHUD)
		{
			GameplayHUD->UpdateLives(GI->GetLives());
			GameplayHUD->UpdateWumpa(GI->GetWumpaCount());
		}
	}
}

void ACBPlayerController::OnLivesChanged(int32 NewLives)
{
	if (GameplayHUD)
	{
		GameplayHUD->UpdateLives(NewLives);
	}
}

void ACBPlayerController::OnWumpaChanged(int32 NewCount)
{
	if (GameplayHUD)
	{
		GameplayHUD->UpdateWumpa(NewCount);
	}
}
