// Unit tests for the Phase 3 pain-chance formula:
//     chance = min(0.8, (damage / max_health) * 2)
//
// From SPEC section 3.3 and Phase 3 automated-test list in section 11.5.
// The formula is extracted as a pure static function on AQuakeEnemyBase
// (mirroring the pattern used by AQuakeCharacter::ApplyArmorAbsorption and
// UQuakeCharacterMovementComponent::ApplyQuakeAirAccel) so these tests
// exercise it directly without spinning up a world, a pawn, or a
// controller.
//
// The other Phase 3 tests listed in SPEC 11.5 (state transitions from sight
// stimuli, hitscan fire on range + LoS, death + unpossess, hearing through
// walls) need a live world + perception system and are better authored as
// AFunctionalTest actors in an in-editor test map, not as unit tests here.
// This file covers the one formula that can be isolated as pure logic.
//
// Per CLAUDE.md "Running Tests": tests are EditorContext + EngineFilter,
// guarded by WITH_DEV_AUTOMATION_TESTS, and live under Source/Quake/Tests.
// Run from Session Frontend -> Automation tab with filter "Quake.*".

#include "Misc/AutomationTest.h"

#include "QuakeEnemyBase.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr float kPainTolerance = UE_KINDA_SMALL_NUMBER;
}

// -----------------------------------------------------------------------------
// Low damage: 10 dmg on a 100 HP enemy.
//     chance = min(0.8, 10/100 * 2) = min(0.8, 0.2) = 0.2
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeEnemyPainChanceLowDamageTest,
	"Quake.Enemy.PainChance.LowDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeEnemyPainChanceLowDamageTest::RunTest(const FString& /*Parameters*/)
{
	const float Chance = AQuakeEnemyBase::ComputePainChance(/*Damage*/ 10.f, /*MaxHealth*/ 100.f);
	TestEqual(TEXT("10 dmg / 100 HP -> 0.2 pain chance"), Chance, 0.2f, kPainTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Right at the 0.8 cap: 40 dmg on a 100 HP enemy.
//     chance = min(0.8, 40/100 * 2) = min(0.8, 0.8) = 0.8
// This is the boundary case — one more unit of damage would still clamp.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeEnemyPainChanceAtCapTest,
	"Quake.Enemy.PainChance.AtCap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeEnemyPainChanceAtCapTest::RunTest(const FString& /*Parameters*/)
{
	const float Chance = AQuakeEnemyBase::ComputePainChance(40.f, 100.f);
	TestEqual(TEXT("40 dmg / 100 HP -> exactly 0.8 (the cap)"), Chance, 0.8f, kPainTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Over the cap: 100 dmg on a 100 HP enemy.
//     chance = min(0.8, 100/100 * 2) = min(0.8, 2.0) = 0.8
// This is the regression test against "someone dropped the min() clamp" —
// if the formula became `2 * damage / max_health` without the min,
// it would return 2.0 and flinch 100% of the time (basically 200%).
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeEnemyPainChanceOverCapTest,
	"Quake.Enemy.PainChance.OverCap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeEnemyPainChanceOverCapTest::RunTest(const FString& /*Parameters*/)
{
	const float Chance = AQuakeEnemyBase::ComputePainChance(100.f, 100.f);
	TestEqual(TEXT("Damage >= max HP clamps chance to 0.8"), Chance, 0.8f, kPainTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Zero damage: no pain roll at all. The controller's code path short-
// circuits on zero anyway, but the helper should agree.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeEnemyPainChanceZeroDamageTest,
	"Quake.Enemy.PainChance.ZeroDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeEnemyPainChanceZeroDamageTest::RunTest(const FString& /*Parameters*/)
{
	const float Chance = AQuakeEnemyBase::ComputePainChance(0.f, 100.f);
	TestEqual(TEXT("Zero damage -> zero pain chance"), Chance, 0.f, kPainTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Divide-by-zero guard: degenerate max-HP input should return 0 rather than
// produce a NaN or INF that then propagates through the FSM.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeEnemyPainChanceZeroMaxHealthTest,
	"Quake.Enemy.PainChance.ZeroMaxHealth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeEnemyPainChanceZeroMaxHealthTest::RunTest(const FString& /*Parameters*/)
{
	const float Chance = AQuakeEnemyBase::ComputePainChance(10.f, 0.f);
	TestEqual(TEXT("Zero MaxHealth -> zero pain chance (no divide-by-zero)"),
		Chance, 0.f, kPainTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// A Grunt-specific sanity check using the exact Grunt MaxHealth (30) and a
// Shotgun-ish damage input (24). Documents the expected in-game chance so
// it's easy to adjust if the formula ever changes.
//     chance = min(0.8, 24/30 * 2) = min(0.8, 1.6) = 0.8
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeEnemyPainChanceGruntVsShotgunTest,
	"Quake.Enemy.PainChance.GruntVsShotgun",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeEnemyPainChanceGruntVsShotgunTest::RunTest(const FString& /*Parameters*/)
{
	// Grunt MaxHealth = 30 (SPEC 3.1); full shotgun blast = 24 dmg.
	const float Chance = AQuakeEnemyBase::ComputePainChance(24.f, 30.f);
	TestEqual(TEXT("Shotgun-to-Grunt clamps to 0.8"), Chance, 0.8f, kPainTolerance);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
