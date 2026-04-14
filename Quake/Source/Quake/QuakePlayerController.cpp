#include "QuakePlayerController.h"

#include "QuakeCharacter.h"
#include "QuakeGameInstance.h"
#include "QuakeHUD.h"
#include "QuakeSaveArchive.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeSaveInput, Log, All);

void AQuakePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMappingContext)
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void AQuakePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Quick-save / quick-load are handled on the controller, not the pawn —
	// they should work from the death screen too (pawn may be gone). Pawn-
	// bound actions (Move/Look/Fire/weapon slots) stay in Character::SetupPlayerInputComponent.
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (QuickSaveAction)
		{
			EnhancedInput->BindAction(QuickSaveAction, ETriggerEvent::Started, this,
				&AQuakePlayerController::OnQuickSavePressed);
		}
		if (QuickLoadAction)
		{
			EnhancedInput->BindAction(QuickLoadAction, ETriggerEvent::Started, this,
				&AQuakePlayerController::OnQuickLoadPressed);
		}
	}
}

void AQuakePlayerController::OnQuickSavePressed(const FInputActionValue& /*Value*/)
{
	UQuakeGameInstance* GI = GetGameInstance<UQuakeGameInstance>();
	if (!GI)
	{
		return;
	}

	// DESIGN 6.2 F5 gate: grounded, not flinching, not dead.
	AQuakeCharacter* Char = Cast<AQuakeCharacter>(GetPawn());
	const EMovementMode Mode = (Char && Char->GetCharacterMovement())
		? Char->GetCharacterMovement()->MovementMode.GetValue()
		: MOVE_None;
	const bool bInPain = Char ? Char->IsInPain() : false;
	const bool bDead   = Char ? Char->IsDead()   : true;

	if (!QuakeSaveArchive::CanQuickSave(Mode, bInPain, bDead))
	{
		UE_LOG(LogQuakeSaveInput, Log, TEXT("QuickSave rejected (mode=%d, pain=%d, dead=%d)"),
			static_cast<int32>(Mode), bInPain, bDead);
		if (AQuakeHUD* HUD = Cast<AQuakeHUD>(GetHUD()))
		{
			HUD->ShowMessage(NSLOCTEXT("Quake", "CantSaveNow", "Can't save right now."), 2.f);
		}
		return;
	}

	const bool bOk = GI->SaveCurrentState(UQuakeGameInstance::BuildQuickSlotName());
	if (AQuakeHUD* HUD = Cast<AQuakeHUD>(GetHUD()))
	{
		HUD->ShowMessage(
			bOk ? NSLOCTEXT("Quake", "GameSaved", "Game saved.")
			    : NSLOCTEXT("Quake", "SaveFailed", "Save failed."),
			1.5f);
	}
}

void AQuakePlayerController::OnQuickLoadPressed(const FInputActionValue& /*Value*/)
{
	if (UQuakeGameInstance* GI = GetGameInstance<UQuakeGameInstance>())
	{
		const bool bOk = GI->LoadFromSlot(UQuakeGameInstance::BuildQuickSlotName());
		if (!bOk)
		{
			if (AQuakeHUD* HUD = Cast<AQuakeHUD>(GetHUD()))
			{
				HUD->ShowMessage(NSLOCTEXT("Quake", "NoSave", "No save to load."), 1.5f);
			}
		}
	}
}
