// Phase 13 unit tests — DESIGN 6.3 / 6.4. The full death → restart loop
// requires a world (PIE), so it lives in the Manual checklist; this file
// covers the pure-static predicates (final-level routing) and the
// inventory-component snapshot round-trip.

#include "Misc/AutomationTest.h"

#include "QuakeAmmoType.h"
#include "QuakeGameMode.h"
#include "QuakeInventoryComponent.h"
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
// DESIGN 6.4 step 3: inventory snapshot round-trip. Serialize the "walked
// in with" state, mutate the live component mid-level, deserialize the
// snapshot onto a fresh component (mimicking the new-pawn respawn path),
// and confirm the values come back byte-identical.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeInventorySnapshotRoundTripTest,
	"Quake.Phase13.InventorySnapshot.RoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeInventorySnapshotRoundTripTest::RunTest(const FString&)
{
	// Live inventory: 100 Yellow armor + 50 shells at level entry.
	UQuakeInventoryComponent* Live = NewObject<UQuakeInventoryComponent>();
	Live->SetArmor(100.f, 0.6f);
	Live->GiveAmmo(EQuakeAmmoType::Shells, 50);

	FQuakeInventorySnapshot LevelEntry;
	Live->SerializeTo(LevelEntry);
	TestTrue(TEXT("Snapshot marked valid on serialize"), LevelEntry.bValid);

	// Mid-level: drain armor + ammo on the same component.
	Live->SetArmor(0.f, 0.f);
	Live->ConsumeAmmo(EQuakeAmmoType::Shells, 49);
	TestEqual(TEXT("Pre-restore shells"), Live->GetAmmo(EQuakeAmmoType::Shells), 1);

	// Death-restart: fresh pawn spawned, fresh component deserializes the
	// saved snapshot. (In production the fresh component is also fresh-allocated.)
	UQuakeInventoryComponent* Respawned = NewObject<UQuakeInventoryComponent>();
	Respawned->DeserializeFrom(LevelEntry);
	TestEqual(TEXT("Armor restored"),       Respawned->GetArmor(),           100.f);
	TestEqual(TEXT("Absorption restored"),  Respawned->GetArmorAbsorption(), 0.6f);
	TestEqual(TEXT("Shells restored to 50"), Respawned->GetAmmo(EQuakeAmmoType::Shells), 50);
	return true;
}

// -----------------------------------------------------------------------------
// Deserializing an invalid (never-captured) snapshot is a no-op shape: the
// call completes without crashing and the unseeded ammo keys are safely
// added via FindOrAdd so GetAmmo returns 0 rather than a missing-key error.
// Mirrors the "fresh game, never walked into a level" edge case.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeInventorySnapshotInvalidRestoreTest,
	"Quake.Phase13.InventorySnapshot.InvalidRestore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeInventorySnapshotInvalidRestoreTest::RunTest(const FString&)
{
	UQuakeInventoryComponent* Inv = NewObject<UQuakeInventoryComponent>();

	FQuakeInventorySnapshot Empty;  // bValid = false by default
	Inv->DeserializeFrom(Empty);

	TestEqual(TEXT("Armor defaults to 0"),       Inv->GetArmor(),           0.f);
	TestEqual(TEXT("Absorption defaults to 0"),  Inv->GetArmorAbsorption(), 0.f);
	TestEqual(TEXT("Shells defaults to 0"),      Inv->GetAmmo(EQuakeAmmoType::Shells), 0);
	TestEqual(TEXT("Nails defaults to 0"),       Inv->GetAmmo(EQuakeAmmoType::Nails),  0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
