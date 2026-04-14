// Phase 12 unit tests — DESIGN 6.1 difficulty multiplier math and table
// lookup. Functional per-enemy verification (Grunt MaxHealth on Easy etc.)
// is covered by manual PIE checks in ROADMAP Phase 12 because the scaling
// happens inside BeginPlay and requires a world.

#include "Misc/AutomationTest.h"

#include "QuakeDifficulty.h"
#include "QuakeDifficultyMultipliers.h"
#include "QuakeEnemyBase.h"
#include "QuakeGameMode.h"

#if WITH_DEV_AUTOMATION_TESTS

// -----------------------------------------------------------------------------
// DESIGN 6.1 table: Easy 0.75/1.0, Normal 1.0/1.0, Hard 1.5/1.25,
// Nightmare 2.0/1.5 + pain-immune.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeDifficultyLookupDefaultsTest,
	"Quake.Difficulty.Lookup.SeededDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeDifficultyLookupDefaultsTest::RunTest(const FString&)
{
	// The GameMode's constructor seeds the table. Instantiate a CDO-like
	// object to inspect the seeded values. (NewObject<AQuakeGameMode>()
	// skips the editor-only world hookups but the constructor still runs.)
	AQuakeGameMode* GM = NewObject<AQuakeGameMode>();
	const TMap<EQuakeDifficulty, FQuakeDifficultyMultipliers>& Table = GM->DifficultyTable;

	const FQuakeDifficultyMultipliers Easy      = AQuakeGameMode::LookupMultipliers(Table, EQuakeDifficulty::Easy);
	const FQuakeDifficultyMultipliers Normal    = AQuakeGameMode::LookupMultipliers(Table, EQuakeDifficulty::Normal);
	const FQuakeDifficultyMultipliers Hard      = AQuakeGameMode::LookupMultipliers(Table, EQuakeDifficulty::Hard);
	const FQuakeDifficultyMultipliers Nightmare = AQuakeGameMode::LookupMultipliers(Table, EQuakeDifficulty::Nightmare);

	TestEqual(TEXT("Easy.EnemyDamage"),  Easy.EnemyDamage,  0.75f);
	TestEqual(TEXT("Easy.EnemyHP"),      Easy.EnemyHP,      1.0f);
	TestFalse(TEXT("Easy pain not immune"), Easy.bSuppressPain);

	TestEqual(TEXT("Normal.EnemyDamage"),  Normal.EnemyDamage,  1.0f);
	TestEqual(TEXT("Normal.EnemyHP"),      Normal.EnemyHP,      1.0f);

	TestEqual(TEXT("Hard.EnemyDamage"),  Hard.EnemyDamage,  1.5f);
	TestEqual(TEXT("Hard.EnemyHP"),      Hard.EnemyHP,      1.25f);

	TestEqual(TEXT("Nightmare.EnemyDamage"),   Nightmare.EnemyDamage,  2.0f);
	TestEqual(TEXT("Nightmare.EnemyHP"),       Nightmare.EnemyHP,      1.5f);
	TestTrue (TEXT("Nightmare pain immune"),   Nightmare.bSuppressPain);
	TestEqual(TEXT("Nightmare ZombieRevive"),  Nightmare.ZombieReviveScale, 0.5f);

	return true;
}

// -----------------------------------------------------------------------------
// Missing key returns a safe default (1.0 / 1.0, no pain immunity).
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeDifficultyLookupMissingTest,
	"Quake.Difficulty.Lookup.MissingReturnsDefault",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeDifficultyLookupMissingTest::RunTest(const FString&)
{
	TMap<EQuakeDifficulty, FQuakeDifficultyMultipliers> Empty;
	const FQuakeDifficultyMultipliers M = AQuakeGameMode::LookupMultipliers(Empty, EQuakeDifficulty::Hard);
	TestEqual(TEXT("EnemyDamage default"), M.EnemyDamage, 1.0f);
	TestEqual(TEXT("EnemyHP default"),     M.EnemyHP,     1.0f);
	TestFalse(TEXT("No pain suppress"),    M.bSuppressPain);
	return true;
}

// -----------------------------------------------------------------------------
// DESIGN 6.1 scaling math: `MaxHealth = Base × EnemyHP`,
// `AttackDamageMultiplier = EnemyDamage`. Validated against the ROADMAP
// Phase 12 acceptance targets: Grunt on Easy → 30, on Hard → 37.5.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeDifficultyScalingMathTest,
	"Quake.Difficulty.Scaling.MathMatchesSpec",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeDifficultyScalingMathTest::RunTest(const FString&)
{
	const float GruntBase = 30.f;

	// Easy: HP × 1.0 = 30, Damage × 0.75.
	{
		FQuakeDifficultyMultipliers M;
		M.EnemyDamage = 0.75f;
		M.EnemyHP     = 1.0f;
		float HP = 0.f, Dmg = 0.f;
		AQuakeEnemyBase::ComputeScaledEnemyStats(GruntBase, M, HP, Dmg);
		TestEqual(TEXT("Grunt HP on Easy"),    HP,  30.f);
		TestEqual(TEXT("Grunt dmg× on Easy"),  Dmg, 0.75f);
	}

	// Hard: HP × 1.25 = 37.5, Damage × 1.5.
	{
		FQuakeDifficultyMultipliers M;
		M.EnemyDamage = 1.5f;
		M.EnemyHP     = 1.25f;
		float HP = 0.f, Dmg = 0.f;
		AQuakeEnemyBase::ComputeScaledEnemyStats(GruntBase, M, HP, Dmg);
		TestEqual(TEXT("Grunt HP on Hard"),    HP,  37.5f);
		TestEqual(TEXT("Grunt dmg× on Hard"),  Dmg, 1.5f);
	}

	// Nightmare: HP × 1.5 = 45, Damage × 2.0.
	{
		FQuakeDifficultyMultipliers M;
		M.EnemyDamage = 2.0f;
		M.EnemyHP     = 1.5f;
		float HP = 0.f, Dmg = 0.f;
		AQuakeEnemyBase::ComputeScaledEnemyStats(GruntBase, M, HP, Dmg);
		TestEqual(TEXT("Grunt HP on Nightmare"),   HP,  45.f);
		TestEqual(TEXT("Grunt dmg× on Nightmare"), Dmg, 2.0f);
	}

	// Degenerate zero HP multiplier does not crash (would make the enemy
	// unspawnable but the math should be well-defined).
	{
		FQuakeDifficultyMultipliers M;
		M.EnemyHP = 0.f;
		float HP = 999.f, Dmg = 999.f;
		AQuakeEnemyBase::ComputeScaledEnemyStats(GruntBase, M, HP, Dmg);
		TestEqual(TEXT("Zero HP multiplier produces zero"), HP, 0.f);
	}

	return true;
}

// -----------------------------------------------------------------------------
// DESIGN 6.1: spawn-count scaling is NOT a multiplier — it's MinDifficulty
// filtering on spawn points. Sanity-check the enum ordering the predicate
// relies on: Easy(0) < Normal(1) < Hard(2) < Nightmare(3).
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeDifficultyEnumOrderingTest,
	"Quake.Difficulty.Enum.OrderingIsMonotonic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeDifficultyEnumOrderingTest::RunTest(const FString&)
{
	TestTrue(TEXT("Easy < Normal"),
		static_cast<uint8>(EQuakeDifficulty::Easy) < static_cast<uint8>(EQuakeDifficulty::Normal));
	TestTrue(TEXT("Normal < Hard"),
		static_cast<uint8>(EQuakeDifficulty::Normal) < static_cast<uint8>(EQuakeDifficulty::Hard));
	TestTrue(TEXT("Hard < Nightmare"),
		static_cast<uint8>(EQuakeDifficulty::Hard) < static_cast<uint8>(EQuakeDifficulty::Nightmare));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
