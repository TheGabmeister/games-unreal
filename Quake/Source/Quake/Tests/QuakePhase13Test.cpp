// Phase 13 unit tests — DESIGN 6.3 / 6.4. The full death → restart loop
// requires a world (PIE), so it lives in the Manual checklist; this file
// covers the pure-static predicates (final-level routing, inventory
// snapshot round-trip).

#include "Misc/AutomationTest.h"

#include "QuakeAmmoType.h"
#include "QuakeGameInstance.h"
#include "QuakeGameMode.h"
#include "QuakeInventorySnapshot.h"

#if WITH_DEV_AUTOMATION_TESTS

// -----------------------------------------------------------------------------
// DESIGN 6.3: final-level exits route to the win screen regardless of the
// NextMapName field. Non-final exits keep their normal NextMap behavior.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeWinScreenRoutingTest,
	"Quake.Phase13.WinRouting.FinalRoutesToWin",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeWinScreenRoutingTest::RunTest(const FString&)
{
	TestTrue (TEXT("Final + valid NextMap routes to win"),
		AQuakeGameMode::ShouldRouteToWinScreen(true,  FName(TEXT("E1M3"))));
	TestTrue (TEXT("Final + empty NextMap routes to win"),
		AQuakeGameMode::ShouldRouteToWinScreen(true,  NAME_None));
	TestFalse(TEXT("Non-final + valid NextMap is normal exit"),
		AQuakeGameMode::ShouldRouteToWinScreen(false, FName(TEXT("E1M2"))));
	TestFalse(TEXT("Non-final + empty NextMap is NOT a win"),
		AQuakeGameMode::ShouldRouteToWinScreen(false, NAME_None));
	return true;
}

// -----------------------------------------------------------------------------
// DESIGN 6.4 step 3: inventory snapshot round-trip. Snapshot at level entry,
// mutate inventory mid-level, restore, confirm the snapshot's values come back.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeInventorySnapshotRoundTripTest,
	"Quake.Phase13.InventorySnapshot.RoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeInventorySnapshotRoundTripTest::RunTest(const FString&)
{
	UQuakeGameInstance* GI = NewObject<UQuakeGameInstance>();
	GI->Init(); // seeds AmmoCounts.

	// Walk in with 50 armor + 50 shells, take Yellow on the way in.
	GI->Armor           = 100.f;
	GI->ArmorAbsorption = 0.6f;
	GI->GiveAmmo(EQuakeAmmoType::Shells, 25); // 25 + 25 = 50 cap-clamped no-op past cap
	GI->SnapshotForLevelEntry();

	// Mid-level: drain armor + ammo.
	GI->Armor           = 0.f;
	GI->ArmorAbsorption = 0.f;
	GI->ConsumeAmmo(EQuakeAmmoType::Shells, 49);
	TestEqual(TEXT("Pre-restore shells"), GI->GetAmmo(EQuakeAmmoType::Shells), 1);

	// Death-restart restores.
	GI->RestoreFromLevelEntrySnapshot();
	TestEqual(TEXT("Armor restored"),       GI->Armor,           100.f);
	TestEqual(TEXT("Absorption restored"),  GI->ArmorAbsorption, 0.6f);
	TestEqual(TEXT("Shells restored to 50"), GI->GetAmmo(EQuakeAmmoType::Shells), 50);
	return true;
}

// -----------------------------------------------------------------------------
// Invalid (never-snapshotted) restore is a no-op — defensive against the case
// where the death flow runs before BeginPlay's snapshot fires.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeInventorySnapshotInvalidNoOpTest,
	"Quake.Phase13.InventorySnapshot.InvalidIsNoOp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeInventorySnapshotInvalidNoOpTest::RunTest(const FString&)
{
	UQuakeGameInstance* GI = NewObject<UQuakeGameInstance>();
	GI->Init();
	GI->Armor = 42.f;
	GI->RestoreFromLevelEntrySnapshot(); // bValid is false
	TestEqual(TEXT("Armor untouched on no-op restore"), GI->Armor, 42.f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
