// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "FF7DialogueTypes.h"
#include "FF7Interactable.h"
#include "FF7NPCActor.h"
#include "FF7PlayerController.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FFF7InteractSpec, "FF7.Interact",
	EAutomationTestFlags::EditorContext
	| EAutomationTestFlags::ClientContext
	| EAutomationTestFlags::ProductFilter)

	UWorld* TransientWorld = nullptr;

	UDataTable* MakeTable()
	{
		UDataTable* Table = NewObject<UDataTable>(TransientWorld);
		Table->RowStruct = FDialogueLineRow::StaticStruct();
		FDialogueLineRow Row1;
		Row1.SpeakerId = FName("Cloud"); Row1.Line = FText::FromString(TEXT("Line one.")); Row1.NextId = FName("L2");
		FDialogueLineRow Row2;
		Row2.SpeakerId = FName("Cloud"); Row2.Line = FText::FromString(TEXT("Line two.")); Row2.NextId = FName("L3");
		FDialogueLineRow Row3;
		Row3.SpeakerId = FName("Cloud"); Row3.Line = FText::FromString(TEXT("Line three.")); Row3.NextId = NAME_None;
		Table->AddRow(FName("L1"), Row1);
		Table->AddRow(FName("L2"), Row2);
		Table->AddRow(FName("L3"), Row3);
		return Table;
	}

END_DEFINE_SPEC(FFF7InteractSpec)

void FFF7InteractSpec::Define()
{
	BeforeEach([this]()
	{
		// InitializeActorsForPlay + BeginPlay are required so Interface UFunction dispatch
		// (ProcessEvent path used by Execute_*) is fully wired in the transient world.
		TransientWorld = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ true);
		TestNotNull(TEXT("Transient world created"), TransientWorld);
		if (TransientWorld)
		{
			TransientWorld->InitializeActorsForPlay(FURL());
			TransientWorld->BeginPlay();
		}
	});

	AfterEach([this]()
	{
		if (TransientWorld)
		{
			TransientWorld->DestroyWorld(false);
			TransientWorld = nullptr;
		}
	});

	Describe("InterfaceDispatch", [this]()
	{
		It("routes Execute_Interact through AFF7NPCActor to controller dialogue state", [this]()
		{
			if (!TransientWorld) return;

			UDataTable* Table = MakeTable();

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AFF7PlayerController* PC = TransientWorld->SpawnActor<AFF7PlayerController>(AFF7PlayerController::StaticClass(), FTransform::Identity, Params);
			AFF7NPCActor* NPC = TransientWorld->SpawnActor<AFF7NPCActor>(AFF7NPCActor::StaticClass(), FTransform::Identity, Params);
			TestNotNull(TEXT("Controller spawned"), PC);
			TestNotNull(TEXT("NPC spawned"), NPC);
			if (!PC || !NPC) return;

			NPC->DialogueTable = Table;
			NPC->StartRowId = FName("L1");

			TestTrue(TEXT("NPC implements IFF7Interactable"),
				NPC->GetClass()->ImplementsInterface(UFF7Interactable::StaticClass()));

			TestFalse(TEXT("Dialogue inactive before interact"), PC->IsDialogueActive());
			IFF7Interactable::Execute_Interact(NPC, PC);
			TestTrue(TEXT("Dialogue active after Execute_Interact"), PC->IsDialogueActive());
			TestEqual(TEXT("At start row"), PC->GetCurrentDialogueRowId(), FName("L1"));
		});
	});

	Describe("DialogueAdvances (in-process)", [this]()
	{
		It("advances line-by-line then closes on NAME_None terminator", [this]()
		{
			if (!TransientWorld) return;

			UDataTable* Table = MakeTable();
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AFF7PlayerController* PC = TransientWorld->SpawnActor<AFF7PlayerController>(AFF7PlayerController::StaticClass(), FTransform::Identity, Params);
			if (!PC) return;

			PC->StartDialogue(Table, FName("L1"));
			TestTrue(TEXT("Active"), PC->IsDialogueActive());

			PC->AdvanceDialogue();
			TestEqual(TEXT("At L2"), PC->GetCurrentDialogueRowId(), FName("L2"));

			PC->AdvanceDialogue();
			TestEqual(TEXT("At L3"), PC->GetCurrentDialogueRowId(), FName("L3"));

			PC->AdvanceDialogue();
			TestFalse(TEXT("Ended after terminal NAME_None"), PC->IsDialogueActive());
			TestEqual(TEXT("RowId cleared"), PC->GetCurrentDialogueRowId(), NAME_None);
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS
