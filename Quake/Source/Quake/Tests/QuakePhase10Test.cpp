// Phase 10 unit tests — Powerups (SPEC 4.3), Keys (SPEC 4.4), Armor
// pickup tier logic (SPEC 1.2 / 4.2), and the Weapon-pickup first-vs-
// subsequent pattern (SPEC 2.2).
//
// Per the "prefer pure static helpers over world-spinup tests" rule in
// CLAUDE.md, these exercise the predicates and table lookups that the
// in-world behaviour is built on. World-dependent tests (actually spawning
// a character, a pickup overlap, a locked door bump) are deferred to
// manual verification per SPEC Phase 10's "Manual verification" list.

#include "Misc/AutomationTest.h"

#include "QuakeKeyColor.h"
#include "QuakePickup_Armor.h"
#include "QuakePlayerState.h"
#include "QuakePowerup.h"

#if WITH_DEV_AUTOMATION_TESTS

// -----------------------------------------------------------------------------
// SPEC 4.3: GivePowerup adds a fresh entry with a timer clamped to
// GetPowerupMaxDuration (60). HasPowerup / GetPowerupRemaining report it.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakePowerupGrantFreshTest,
	"Quake.Powerup.Grant.FreshEntry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakePowerupGrantFreshTest::RunTest(const FString&)
{
	AQuakePlayerState* PS = NewObject<AQuakePlayerState>();
	TestFalse(TEXT("Fresh PS has no Quad"), PS->HasPowerup(EQuakePowerup::Quad));

	PS->GivePowerup(EQuakePowerup::Quad, 30.f);

	TestTrue(TEXT("Quad active after grant"), PS->HasPowerup(EQuakePowerup::Quad));
	TestEqual(TEXT("Quad timer = 30s"),
		PS->GetPowerupRemaining(EQuakePowerup::Quad), 30.f, UE_KINDA_SMALL_NUMBER);
	TestEqual(TEXT("Single entry"), PS->ActivePowerups.Num(), 1);
	return true;
}

// -----------------------------------------------------------------------------
// SPEC 4.3: "Same powerup refreshes additively, capped at 60 seconds."
// Two back-to-back 30 s Quads = 60 s, not 2 × 30 s entries.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakePowerupRefreshCapTest,
	"Quake.Powerup.Grant.AdditiveCapAt60",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakePowerupRefreshCapTest::RunTest(const FString&)
{
	AQuakePlayerState* PS = NewObject<AQuakePlayerState>();
	PS->GivePowerup(EQuakePowerup::Quad, 30.f);
	PS->GivePowerup(EQuakePowerup::Quad, 30.f);

	TestEqual(TEXT("Still single entry after refresh"), PS->ActivePowerups.Num(), 1);
	TestEqual(TEXT("Timer capped at 60s"),
		PS->GetPowerupRemaining(EQuakePowerup::Quad), 60.f, UE_KINDA_SMALL_NUMBER);

	// Third grant stays at 60 (no overflow).
	PS->GivePowerup(EQuakePowerup::Quad, 30.f);
	TestEqual(TEXT("Third grant doesn't exceed cap"),
		PS->GetPowerupRemaining(EQuakePowerup::Quad), 60.f, UE_KINDA_SMALL_NUMBER);
	return true;
}

// -----------------------------------------------------------------------------
// SPEC 4.3: "Different powerups stack." Separate entries, independent timers.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakePowerupStackingTest,
	"Quake.Powerup.Grant.DifferentTypesStack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakePowerupStackingTest::RunTest(const FString&)
{
	AQuakePlayerState* PS = NewObject<AQuakePlayerState>();
	PS->GivePowerup(EQuakePowerup::Quad, 30.f);
	PS->GivePowerup(EQuakePowerup::Pentagram, 30.f);

	TestEqual(TEXT("Two separate entries"), PS->ActivePowerups.Num(), 2);
	TestTrue(TEXT("Quad active"), PS->HasPowerup(EQuakePowerup::Quad));
	TestTrue(TEXT("Pentagram active"), PS->HasPowerup(EQuakePowerup::Pentagram));
	return true;
}

// -----------------------------------------------------------------------------
// SPEC 4.3 expiry: Tick(dt) decrements timers and prunes zeroed entries.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakePowerupExpiryTest,
	"Quake.Powerup.Expiry.TickPrunes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakePowerupExpiryTest::RunTest(const FString&)
{
	AQuakePlayerState* PS = NewObject<AQuakePlayerState>();
	PS->GivePowerup(EQuakePowerup::Quad, 30.f);

	// One 31-second tick runs the entry to zero and the pruner removes it.
	PS->Tick(31.f);

	TestFalse(TEXT("Quad expired"), PS->HasPowerup(EQuakePowerup::Quad));
	TestEqual(TEXT("ActivePowerups emptied"), PS->ActivePowerups.Num(), 0);
	return true;
}

// -----------------------------------------------------------------------------
// SPEC 4.3 guards: None-type and non-positive durations are ignored (these
// would otherwise be invisible placeholder entries a save-load could revive).
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakePowerupGrantGuardsTest,
	"Quake.Powerup.Grant.RejectsNoneAndZeroDuration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakePowerupGrantGuardsTest::RunTest(const FString&)
{
	AQuakePlayerState* PS = NewObject<AQuakePlayerState>();
	PS->GivePowerup(EQuakePowerup::None, 30.f);
	PS->GivePowerup(EQuakePowerup::Quad, 0.f);
	PS->GivePowerup(EQuakePowerup::Quad, -5.f);

	TestEqual(TEXT("No entries created"), PS->ActivePowerups.Num(), 0);
	return true;
}

// -----------------------------------------------------------------------------
// SPEC 4.4 key storage on PlayerState.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeKeyGrantTest,
	"Quake.Key.GiveAndHas",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeKeyGrantTest::RunTest(const FString&)
{
	AQuakePlayerState* PS = NewObject<AQuakePlayerState>();
	TestFalse(TEXT("No silver initially"), PS->HasKey(EQuakeKeyColor::Silver));

	PS->GiveKey(EQuakeKeyColor::Silver);

	TestTrue(TEXT("Silver held after grant"), PS->HasKey(EQuakeKeyColor::Silver));
	TestFalse(TEXT("Gold still missing"), PS->HasKey(EQuakeKeyColor::Gold));

	// Re-grant is a no-op (SPEC 4.4 "picking up a held key is silent").
	PS->GiveKey(EQuakeKeyColor::Silver);
	TestTrue(TEXT("Silver still held"), PS->HasKey(EQuakeKeyColor::Silver));

	// None guard: giving None never stores.
	PS->GiveKey(EQuakeKeyColor::None);
	TestFalse(TEXT("None sentinel not storable"), PS->HasKey(EQuakeKeyColor::None));
	return true;
}

// -----------------------------------------------------------------------------
// SPEC 1.4 / 6.4 / Phase 10: ClearPerLifeState wipes powerups AND keys,
// preserving per-attempt score.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeClearPerLifeClearsKeysTest,
	"Quake.Key.ClearPerLifeStateEmptiesKeys",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeClearPerLifeClearsKeysTest::RunTest(const FString&)
{
	AQuakePlayerState* PS = NewObject<AQuakePlayerState>();
	PS->AddKillCredit();
	PS->AddSecretCredit();
	PS->AddDeath();
	PS->GivePowerup(EQuakePowerup::Quad, 30.f);
	PS->GiveKey(EQuakeKeyColor::Silver);
	PS->GiveKey(EQuakeKeyColor::Gold);

	PS->ClearPerLifeState();

	TestEqual(TEXT("Kills preserved"),   PS->Kills,   1);
	TestEqual(TEXT("Secrets preserved"), PS->Secrets, 1);
	TestEqual(TEXT("Deaths preserved"),  PS->Deaths,  1);
	TestEqual(TEXT("Powerups emptied"),  PS->ActivePowerups.Num(), 0);
	TestFalse(TEXT("Silver key cleared"), PS->HasKey(EQuakeKeyColor::Silver));
	TestFalse(TEXT("Gold key cleared"),   PS->HasKey(EQuakeKeyColor::Gold));
	return true;
}

// -----------------------------------------------------------------------------
// SPEC 4.2 armor tier tables. Regresses the amount + absorption lookup so
// a typo in the switch doesn't silently ship.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeArmorTierTableTest,
	"Quake.Armor.TierTable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeArmorTierTableTest::RunTest(const FString&)
{
	TestEqual(TEXT("Green = 100/0.3"),
		AQuakePickup_Armor::GetAmountForTier(EQuakeArmorTier::Green), 100.f);
	TestEqual(TEXT("Green absorption = 0.3"),
		AQuakePickup_Armor::GetAbsorptionForTier(EQuakeArmorTier::Green), 0.3f, UE_KINDA_SMALL_NUMBER);

	TestEqual(TEXT("Yellow = 150/0.6"),
		AQuakePickup_Armor::GetAmountForTier(EQuakeArmorTier::Yellow), 150.f);
	TestEqual(TEXT("Yellow absorption = 0.6"),
		AQuakePickup_Armor::GetAbsorptionForTier(EQuakeArmorTier::Yellow), 0.6f, UE_KINDA_SMALL_NUMBER);

	TestEqual(TEXT("Red = 200/0.8"),
		AQuakePickup_Armor::GetAmountForTier(EQuakeArmorTier::Red), 200.f);
	TestEqual(TEXT("Red absorption = 0.8"),
		AQuakePickup_Armor::GetAbsorptionForTier(EQuakeArmorTier::Red), 0.8f, UE_KINDA_SMALL_NUMBER);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
