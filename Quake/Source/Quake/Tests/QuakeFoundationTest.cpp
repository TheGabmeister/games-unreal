// SPEC Phase 0 deliverable: a trivial automation test that proves the test
// runner picks up Quake-module tests under the "Quake.*" filter.
//
// This test is intentionally trivial — its only job is to confirm that the
// module's test infrastructure (Tests/ subdirectory, PrivateIncludePaths,
// WITH_DEV_AUTOMATION_TESTS guards, IMPLEMENT_SIMPLE_AUTOMATION_TEST flags)
// all work end-to-end. Real logic tests live alongside the code they cover
// (see QuakeCharacterMovementComponentTest.cpp).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeFoundationTest,
	"Quake.Foundation.Smoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeFoundationTest::RunTest(const FString& /*Parameters*/)
{
	TestTrue(TEXT("Quake module test runner is reachable"), true);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
