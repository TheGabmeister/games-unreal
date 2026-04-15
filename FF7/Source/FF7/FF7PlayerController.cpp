// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF7PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "FF7CollisionChannels.h"
#include "FF7DialogueTypes.h"
#include "FF7Interactable.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "UI/SFF7DialoguePopup.h"
#include "Widgets/SWeakWidget.h"

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
	if (UInputAction* InteractAction = IA_Interact.LoadSynchronous())
	{
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AFF7PlayerController::HandleInteract);
	}
	// IA_MenuToggle / IA_Escape bindings land in later phases.
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

void AFF7PlayerController::HandleInteract(const FInputActionValue& /*Value*/)
{
	if (bDialogueActive)
	{
		AdvanceDialogue();
		return;
	}

	if (AActor* Target = FindInteractable())
	{
		if (Target->GetClass()->ImplementsInterface(UFF7Interactable::StaticClass()))
		{
			IFF7Interactable::Execute_Interact(Target, this);
		}
	}
}

AActor* AFF7PlayerController::FindInteractable() const
{
	APawn* ControlledPawn = GetPawn();
	UWorld* World = GetWorld();
	if (!ControlledPawn || !World)
	{
		return nullptr;
	}

	const FVector Start = ControlledPawn->GetActorLocation();
	const FVector Forward = ControlledPawn->GetActorForwardVector();
	const FVector End = Start + Forward * InteractTraceDistance;

	// Primary: forward line trace on the custom Interact channel.
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FF7Interact), /*bTraceComplex*/ false, ControlledPawn);
	if (World->LineTraceSingleByChannel(Hit, Start, End, FF7::ECC_Interact, Params))
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			if (HitActor->GetClass()->ImplementsInterface(UFF7Interactable::StaticClass()))
			{
				return HitActor;
			}
		}
	}

	// Fallback: closest overlapping actor implementing IFF7Interactable.
	TArray<AActor*> Overlapping;
	ControlledPawn->GetOverlappingActors(Overlapping);
	AActor* BestActor = nullptr;
	float BestDistSq = FLT_MAX;
	for (AActor* Actor : Overlapping)
	{
		if (!Actor || !Actor->GetClass()->ImplementsInterface(UFF7Interactable::StaticClass()))
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(Actor->GetActorLocation(), Start);
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestActor = Actor;
		}
	}
	return BestActor;
}

void AFF7PlayerController::StartDialogue(UDataTable* Table, FName StartRowId)
{
	if (!Table || StartRowId.IsNone())
	{
		return;
	}

	CurrentDialogueTable = Table;
	CurrentRowId = StartRowId;
	bDialogueActive = true;
	LoadCurrentRow();

	if (!DialoguePopup.IsValid())
	{
		DialoguePopup = SNew(SFF7DialoguePopup)
			.SpeakerText_Lambda([this]() { return CachedSpeakerText; })
			.LineText_Lambda([this]() { return CachedLineText; });

		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->AddViewportWidgetContent(DialoguePopup.ToSharedRef());
		}
	}
}

void AFF7PlayerController::AdvanceDialogue()
{
	if (!bDialogueActive || !CurrentDialogueTable)
	{
		return;
	}

	const FDialogueLineRow* Row = CurrentDialogueTable->FindRow<FDialogueLineRow>(CurrentRowId, TEXT("FF7 AdvanceDialogue"));
	if (!Row || Row->NextId.IsNone())
	{
		EndDialogue();
		return;
	}

	CurrentRowId = Row->NextId;
	LoadCurrentRow();
}

void AFF7PlayerController::EndDialogue()
{
	if (DialoguePopup.IsValid())
	{
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(DialoguePopup.ToSharedRef());
		}
		DialoguePopup.Reset();
	}

	CurrentDialogueTable = nullptr;
	CurrentRowId = NAME_None;
	CachedSpeakerText = FText::GetEmpty();
	CachedLineText = FText::GetEmpty();
	bDialogueActive = false;
}

void AFF7PlayerController::LoadCurrentRow()
{
	if (!CurrentDialogueTable)
	{
		return;
	}

	const FDialogueLineRow* Row = CurrentDialogueTable->FindRow<FDialogueLineRow>(CurrentRowId, TEXT("FF7 LoadCurrentRow"));
	if (!Row)
	{
		CachedSpeakerText = FText::GetEmpty();
		CachedLineText = FText::GetEmpty();
		return;
	}

	CachedSpeakerText = FText::FromName(Row->SpeakerId);
	CachedLineText = Row->Line;
}
