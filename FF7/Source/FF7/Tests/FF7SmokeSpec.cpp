// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FFF7SmokeSpec, "FF7.Smoke.TestHarness",
	EAutomationTestFlags::EditorContext
	| EAutomationTestFlags::ClientContext
	| EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FFF7SmokeSpec)

void FFF7SmokeSpec::Define()
{
	Describe("Test harness", [this]()
	{
		It("compiles and registers", [this]()
		{
			TestTrue(TEXT("Automation harness reached"), true);
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS
