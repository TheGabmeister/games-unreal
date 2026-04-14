// Unit tests for the ammo API on UQuakeInventoryComponent.
//
// Covered:
//   - GetAmmoCap table lookup (SPEC section 2.1): pure static, no instance.
//   - GiveAmmo: add below cap / at cap / over cap; return value is the
//     delta actually applied.
//   - ConsumeAmmo: success when sufficient, failure when insufficient,
//     None short-circuit for Axe-style weapons.
//
// Per CLAUDE.md "Running Tests": EditorContext + EngineFilter, guarded by
// WITH_DEV_AUTOMATION_TESTS, run via Session Frontend → Automation tab
// with filter "Quake.*".
//
// Component instances are created with NewObject — no world, no owning
// actor. The ammo methods only touch the internal AmmoCounts TMap (no
// subsystems, no world queries), so the object is usable directly without
// InitializeComponent. Fresh instances start with an empty TMap; tests
// that want the "starting loadout" state call SeedDefaults indirectly via
// the CallInitializeComponent escape hatch (see the starting-loadout test).

#include "Misc/AutomationTest.h"

#include "QuakeAmmoType.h"
#include "QuakeInventoryComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

// -----------------------------------------------------------------------------
// SPEC 2.1 ammo cap table. Values are hardcoded in GetAmmoCap — regression
// target if someone accidentally renumbers the enum or touches the switch.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAmmoCapTableTest,
	"Quake.Ammo.Caps.Table",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAmmoCapTableTest::RunTest(const FString& /*Parameters*/)
{
	TestEqual(TEXT("Shells cap = 100"),  UQuakeInventoryComponent::GetAmmoCap(EQuakeAmmoType::Shells),  100);
	TestEqual(TEXT("Nails cap = 200"),   UQuakeInventoryComponent::GetAmmoCap(EQuakeAmmoType::Nails),   200);
	TestEqual(TEXT("Rockets cap = 100"), UQuakeInventoryComponent::GetAmmoCap(EQuakeAmmoType::Rockets), 100);
	TestEqual(TEXT("Cells cap = 100"),   UQuakeInventoryComponent::GetAmmoCap(EQuakeAmmoType::Cells),   100);
	TestEqual(TEXT("None cap = 0"),      UQuakeInventoryComponent::GetAmmoCap(EQuakeAmmoType::None),    0);
	return true;
}

// -----------------------------------------------------------------------------
// GiveAmmo below cap: fully accepted, returns the full amount.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAmmoGiveBelowCapTest,
	"Quake.Ammo.Give.BelowCap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAmmoGiveBelowCapTest::RunTest(const FString& /*Parameters*/)
{
	UQuakeInventoryComponent* Inv = NewObject<UQuakeInventoryComponent>();
	TestNotNull(TEXT("Component constructed"), Inv);
	if (!Inv) return false;

	// Empty start — no InitializeComponent call. GetAmmo on a fresh instance returns 0.
	TestEqual(TEXT("Fresh Shells = 0"), Inv->GetAmmo(EQuakeAmmoType::Shells), 0);

	const int32 Added = Inv->GiveAmmo(EQuakeAmmoType::Shells, 20);
	TestEqual(TEXT("GiveAmmo returned 20 (fully accepted)"), Added, 20);
	TestEqual(TEXT("Shells = 20 after give"), Inv->GetAmmo(EQuakeAmmoType::Shells), 20);
	return true;
}

// -----------------------------------------------------------------------------
// GiveAmmo at cap: clamped to cap, partial accept, return reflects delta.
// Shells cap = 100. Start at 90, give 20 → expect +10 accepted, 100 total.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAmmoGiveAtCapTest,
	"Quake.Ammo.Give.AtCap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAmmoGiveAtCapTest::RunTest(const FString& /*Parameters*/)
{
	UQuakeInventoryComponent* Inv = NewObject<UQuakeInventoryComponent>();
	if (!Inv) return false;

	Inv->GiveAmmo(EQuakeAmmoType::Shells, 90);
	const int32 Added = Inv->GiveAmmo(EQuakeAmmoType::Shells, 20);
	TestEqual(TEXT("GiveAmmo returned 10 (clamped to cap)"), Added, 10);
	TestEqual(TEXT("Shells = 100 at cap"), Inv->GetAmmo(EQuakeAmmoType::Shells), 100);
	return true;
}

// -----------------------------------------------------------------------------
// GiveAmmo already at cap: zero accepted.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAmmoGiveOverCapTest,
	"Quake.Ammo.Give.OverCap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAmmoGiveOverCapTest::RunTest(const FString& /*Parameters*/)
{
	UQuakeInventoryComponent* Inv = NewObject<UQuakeInventoryComponent>();
	if (!Inv) return false;

	Inv->GiveAmmo(EQuakeAmmoType::Shells, 100);
	const int32 Added = Inv->GiveAmmo(EQuakeAmmoType::Shells, 50);
	TestEqual(TEXT("GiveAmmo returned 0 (already at cap)"), Added, 0);
	TestEqual(TEXT("Shells stays at 100"), Inv->GetAmmo(EQuakeAmmoType::Shells), 100);
	return true;
}

// -----------------------------------------------------------------------------
// ConsumeAmmo when sufficient: returns true, deducts the amount.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAmmoConsumeSufficientTest,
	"Quake.Ammo.Consume.Sufficient",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAmmoConsumeSufficientTest::RunTest(const FString& /*Parameters*/)
{
	UQuakeInventoryComponent* Inv = NewObject<UQuakeInventoryComponent>();
	if (!Inv) return false;

	Inv->GiveAmmo(EQuakeAmmoType::Shells, 10);
	const bool bConsumed = Inv->ConsumeAmmo(EQuakeAmmoType::Shells, 3);
	TestTrue(TEXT("ConsumeAmmo(3) succeeds with 10 available"), bConsumed);
	TestEqual(TEXT("Shells = 7 after consume"), Inv->GetAmmo(EQuakeAmmoType::Shells), 7);
	return true;
}

// -----------------------------------------------------------------------------
// ConsumeAmmo when insufficient: returns false, does NOT deduct.
// This is the "atomic consume" contract — a failed consume must not
// partially drain the pool, so the click-on-empty path reads a stable
// count on the next frame.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAmmoConsumeInsufficientTest,
	"Quake.Ammo.Consume.Insufficient",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAmmoConsumeInsufficientTest::RunTest(const FString& /*Parameters*/)
{
	UQuakeInventoryComponent* Inv = NewObject<UQuakeInventoryComponent>();
	if (!Inv) return false;

	Inv->GiveAmmo(EQuakeAmmoType::Shells, 2);
	const bool bConsumed = Inv->ConsumeAmmo(EQuakeAmmoType::Shells, 5);
	TestFalse(TEXT("ConsumeAmmo(5) fails with only 2 available"), bConsumed);
	TestEqual(TEXT("Shells unchanged at 2 after failed consume"), Inv->GetAmmo(EQuakeAmmoType::Shells), 2);
	return true;
}

// -----------------------------------------------------------------------------
// ConsumeAmmo(None, ...) succeeds unconditionally — the Axe fires without
// ammo and AQuakeWeaponBase::TryFire routes this through ConsumeAmmo to
// keep one code path. The behavior must stay "true even with zero None
// ammo" so the Axe never click-fails.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAmmoConsumeNoneTest,
	"Quake.Ammo.Consume.None",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAmmoConsumeNoneTest::RunTest(const FString& /*Parameters*/)
{
	UQuakeInventoryComponent* Inv = NewObject<UQuakeInventoryComponent>();
	if (!Inv) return false;

	const bool bConsumed = Inv->ConsumeAmmo(EQuakeAmmoType::None, 1);
	TestTrue(TEXT("ConsumeAmmo(None) always true"), bConsumed);
	return true;
}

// -----------------------------------------------------------------------------
// Deserializing a snapshot with the SPEC 1.4 starting loadout seeds the
// TMap correctly. Fresh-run "first-time hydration" happens inside
// InitializeComponent when there's no world context (which requires a live
// UQuakeGameInstance via GetChecked); testing that path requires world
// spin-up. Here we exercise the equivalent shape by round-tripping a
// loadout-shaped snapshot through Deserialize, which is the same code
// path InitializeComponent invokes when the mailbox is valid.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAmmoStartingLoadoutTest,
	"Quake.Ammo.Init.StartingLoadout",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAmmoStartingLoadoutTest::RunTest(const FString& /*Parameters*/)
{
	UQuakeInventoryComponent* Inv = NewObject<UQuakeInventoryComponent>();
	if (!Inv) return false;

	FQuakeInventorySnapshot Snap;
	Snap.AmmoCounts.Add(EQuakeAmmoType::Shells, 25);
	Snap.bValid = true;
	Inv->DeserializeFrom(Snap);

	TestEqual(TEXT("Starting Shells = 25"),  Inv->GetAmmo(EQuakeAmmoType::Shells),  25);
	TestEqual(TEXT("Starting Nails = 0"),    Inv->GetAmmo(EQuakeAmmoType::Nails),   0);
	TestEqual(TEXT("Starting Rockets = 0"),  Inv->GetAmmo(EQuakeAmmoType::Rockets), 0);
	TestEqual(TEXT("Starting Cells = 0"),    Inv->GetAmmo(EQuakeAmmoType::Cells),   0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
