// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "FF7SmokeFunctionalTest.generated.h"

/**
 * AFF7SmokeFunctionalTest — drop a single instance into L_TestChamber.
 *
 * Session Frontend → Automation → FF7.Smoke.TestMapLoads runs a 10-frame
 * tick, then calls FinishTest(Succeeded). Passes if the map loads and
 * ticks without errors.
 */
UCLASS()
class AFF7SmokeFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	AFF7SmokeFunctionalTest();

protected:
	virtual void StartTest() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	int32 TicksObserved = 0;
	static constexpr int32 RequiredTicks = 10;
};
