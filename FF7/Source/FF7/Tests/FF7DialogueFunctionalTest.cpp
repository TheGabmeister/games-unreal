// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF7DialogueFunctionalTest.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "FF7DialogueTypes.h"
#include "FF7Interactable.h"
#include "FF7NPCActor.h"
#include "FF7PlayerController.h"
#include "Kismet/GameplayStatics.h"

AFF7DialogueFunctionalTest::AFF7DialogueFunctionalTest()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	TimeLimit = 5.0f;
}

void AFF7DialogueFunctionalTest::StartTest()
{
	Super::StartTest();
	Step = 0;

	UWorld* World = GetWorld();
	if (!World)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("No world."));
		return;
	}

	Table = NewObject<UDataTable>(this);
	Table->RowStruct = FDialogueLineRow::StaticStruct();
	FDialogueLineRow R1;
	R1.SpeakerId = FName("Cloud"); R1.Line = FText::FromString(TEXT("Hello.")); R1.NextId = FName("L2");
	FDialogueLineRow R2;
	R2.SpeakerId = FName("Cloud"); R2.Line = FText::FromString(TEXT("Goodbye.")); R2.NextId = NAME_None;
	Table->AddRow(FName("L1"), R1);
	Table->AddRow(FName("L2"), R2);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnedNPC = World->SpawnActor<AFF7NPCActor>(AFF7NPCActor::StaticClass(), GetActorTransform(), Params);
	if (!SpawnedNPC)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("NPC spawn failed."));
		return;
	}
	SpawnedNPC->DialogueTable = Table;
	SpawnedNPC->StartRowId = FName("L1");

	Controller = Cast<AFF7PlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (!Controller)
	{
		// Fall back: spawn a controller directly so the test works in a map without BP_FF7FieldGameMode.
		Controller = World->SpawnActor<AFF7PlayerController>(AFF7PlayerController::StaticClass(), FTransform::Identity, Params);
	}
	if (!Controller)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Controller unavailable."));
		return;
	}
}

void AFF7DialogueFunctionalTest::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsRunning() || !Controller || !SpawnedNPC)
	{
		return;
	}

	switch (Step)
	{
	case 0:
		IFF7Interactable::Execute_Interact(SpawnedNPC, Controller);
		if (!Controller->IsDialogueActive() || Controller->GetCurrentDialogueRowId() != FName("L1"))
		{
			FinishTest(EFunctionalTestResult::Failed, TEXT("Expected dialogue at L1 after interact."));
			return;
		}
		++Step;
		break;

	case 1:
		Controller->AdvanceDialogue();
		if (Controller->GetCurrentDialogueRowId() != FName("L2"))
		{
			FinishTest(EFunctionalTestResult::Failed, TEXT("Expected dialogue at L2 after advance."));
			return;
		}
		++Step;
		break;

	case 2:
		Controller->AdvanceDialogue();
		if (Controller->IsDialogueActive())
		{
			FinishTest(EFunctionalTestResult::Failed, TEXT("Dialogue should have ended on NAME_None."));
			return;
		}
		FinishTest(EFunctionalTestResult::Succeeded, TEXT("Dialogue advanced L1→L2→end."));
		return;
	}
}
