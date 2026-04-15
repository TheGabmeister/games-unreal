// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF7SmokeFunctionalTest.h"

AFF7SmokeFunctionalTest::AFF7SmokeFunctionalTest()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	TimeLimit = 5.0f;
}

void AFF7SmokeFunctionalTest::StartTest()
{
	Super::StartTest();
	TicksObserved = 0;
}

void AFF7SmokeFunctionalTest::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsRunning())
	{
		return;
	}

	if (++TicksObserved >= RequiredTicks)
	{
		FinishTest(EFunctionalTestResult::Succeeded, TEXT("Map ticked 10 frames."));
	}
}
