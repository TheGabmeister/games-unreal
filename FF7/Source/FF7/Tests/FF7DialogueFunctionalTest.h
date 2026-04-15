// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "FF7DialogueFunctionalTest.generated.h"

class AFF7NPCActor;
class AFF7PlayerController;
class UDataTable;

/**
 * FF7.Interact.DialogueAdvances — place in any map that has a PlayerStart.
 * Spawns a DataTable + NPC + controller, routes Execute_Interact through
 * the NPC, then advances to the terminator and verifies cleanup. Runs in
 * PIE so the viewport widget path is exercised, unlike the headless spec.
 */
UCLASS()
class AFF7DialogueFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	AFF7DialogueFunctionalTest();

protected:
	virtual void StartTest() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY()
	TObjectPtr<UDataTable> Table;

	UPROPERTY()
	TObjectPtr<AFF7NPCActor> SpawnedNPC;

	UPROPERTY()
	TObjectPtr<AFF7PlayerController> Controller;

	int32 Step = 0;
};
