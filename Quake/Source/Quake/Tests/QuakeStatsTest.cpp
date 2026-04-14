// Phase 9 unit tests — SPEC sections 5.1 / 5.9 (spawn-point eligibility,
// satisfaction, and level-clear scan).
//
// Functional coverage that requires a live world (overlap-driven secret
// crediting, the StatsSandbox kills/secrets/exit flow, the level-end
// stats screen) is deferred to manual verification per the same trade-off
// already documented in QuakeLevelStructureTest.cpp. These unit tests
// exercise the pure / world-free predicates that the in-world flow is
// built on, so a regression in the math trips here before the sandbox.

#include "Misc/AutomationTest.h"

#include "QuakeDifficulty.h"
#include "QuakeEnemy_Grunt.h"
#include "QuakeEnemyBase.h"
#include "QuakeEnemySpawnPoint.h"
#include "QuakeGameMode.h"
#include "QuakePlayerState.h"
#include "QuakePowerup.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	// Construct a transient spawn point. NewObject on a UCLASS works without
	// a world for state-only inspection — none of these tests call BeginPlay
	// or SpawnActor, so the spawn-point's TrySpawn path never runs.
	AQuakeEnemySpawnPoint* MakeSpawnPoint(
		bool bMarked,
		EQuakeDifficulty MinDifficulty,
		bool bDeferred)
	{
		AQuakeEnemySpawnPoint* SP = NewObject<AQuakeEnemySpawnPoint>();
		SP->bIsMarkedKillTarget = bMarked;
		SP->MinDifficulty = MinDifficulty;
		SP->bDeferredSpawn = bDeferred;
		return SP;
	}

	AQuakeEnemyBase* MakeEnemyAtHealth(float Health)
	{
		// AQuakeEnemyBase is abstract (PURE_VIRTUAL FireAtTarget); NewObject
		// the Grunt concrete leaf so reflection lets us instantiate.
		AQuakeEnemyBase* E = NewObject<AQuakeEnemy_Grunt>();
		E->SetHealthForTest(Health);
		return E;
	}
}

// -----------------------------------------------------------------------------
// AQuakeEnemySpawnPoint::IsEligibleForDifficulty — SPEC 5.1.
// "Easy = always; Nightmare = nightmare-only" — i.e. the predicate is
// CurrentDifficulty >= MinDifficulty AND bIsMarkedKillTarget.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsSpawnPointEligibleMarkedEasyTest,
	"Quake.Stats.SpawnPoint.Eligible.MarkedEasyAtNormal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsSpawnPointEligibleMarkedEasyTest::RunTest(const FString&)
{
	AQuakeEnemySpawnPoint* SP = MakeSpawnPoint(/*bMarked*/true, EQuakeDifficulty::Easy, /*bDeferred*/false);
	TestTrue(TEXT("Marked Easy spawn point eligible at Normal"),
		SP->IsEligibleForDifficulty(EQuakeDifficulty::Normal));
	TestTrue(TEXT("Marked Easy spawn point eligible at Nightmare"),
		SP->IsEligibleForDifficulty(EQuakeDifficulty::Nightmare));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsSpawnPointEligibleNightmareGateTest,
	"Quake.Stats.SpawnPoint.Eligible.NightmareOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsSpawnPointEligibleNightmareGateTest::RunTest(const FString&)
{
	AQuakeEnemySpawnPoint* SP = MakeSpawnPoint(true, EQuakeDifficulty::Nightmare, false);
	TestFalse(TEXT("Nightmare-only ineligible at Easy"),
		SP->IsEligibleForDifficulty(EQuakeDifficulty::Easy));
	TestFalse(TEXT("Nightmare-only ineligible at Hard"),
		SP->IsEligibleForDifficulty(EQuakeDifficulty::Hard));
	TestTrue(TEXT("Nightmare-only eligible at Nightmare"),
		SP->IsEligibleForDifficulty(EQuakeDifficulty::Nightmare));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsSpawnPointUnmarkedTest,
	"Quake.Stats.SpawnPoint.Eligible.UnmarkedExcluded",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsSpawnPointUnmarkedTest::RunTest(const FString&)
{
	// Decoration: bIsMarkedKillTarget = false. Should never be eligible at
	// any difficulty per SPEC 5.1.
	AQuakeEnemySpawnPoint* SP = MakeSpawnPoint(/*bMarked*/false, EQuakeDifficulty::Easy, false);
	TestFalse(TEXT("Unmarked decoration ineligible at Normal"),
		SP->IsEligibleForDifficulty(EQuakeDifficulty::Normal));
	TestFalse(TEXT("Unmarked decoration ineligible at Nightmare"),
		SP->IsEligibleForDifficulty(EQuakeDifficulty::Nightmare));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsSpawnPointDeferredCountsTest,
	"Quake.Stats.SpawnPoint.Eligible.DeferredStillCounts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsSpawnPointDeferredCountsTest::RunTest(const FString&)
{
	// Deferred spawn points still owe an enemy to KillsTotal — the deferred
	// flag controls when the enemy spawns, not whether it counts. SPEC 5.9.
	AQuakeEnemySpawnPoint* SP = MakeSpawnPoint(true, EQuakeDifficulty::Easy, /*bDeferred*/true);
	TestTrue(TEXT("Deferred marked spawn point still eligible"),
		SP->IsEligibleForDifficulty(EQuakeDifficulty::Normal));
	return true;
}

// -----------------------------------------------------------------------------
// AQuakeEnemySpawnPoint::IsSatisfied — SPEC 5.9.
// "spawned + dead". Unspawned (no SpawnedEnemy) is never satisfied even
// when eligible — that's how deferred spawn points keep the level unclear
// until their trigger fires.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsSpawnPointSatisfiedTest,
	"Quake.Stats.SpawnPoint.Satisfied.States",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsSpawnPointSatisfiedTest::RunTest(const FString&)
{
	AQuakeEnemySpawnPoint* SP = MakeSpawnPoint(true, EQuakeDifficulty::Easy, false);
	TestFalse(TEXT("Unspawned spawn point not satisfied"), SP->IsSatisfied());

	AQuakeEnemyBase* Alive = MakeEnemyAtHealth(30.f);
	SP->SpawnedEnemy = Alive;
	TestFalse(TEXT("Spawned but alive not satisfied"), SP->IsSatisfied());

	AQuakeEnemyBase* Dead = MakeEnemyAtHealth(0.f);
	SP->SpawnedEnemy = Dead;
	TestTrue(TEXT("Spawned and dead is satisfied"), SP->IsSatisfied());
	return true;
}

// -----------------------------------------------------------------------------
// AQuakeGameMode::IsLevelClearedForSet — SPEC 5.9.
// Pure helper version of the in-world TActorIterator scan. Empty set is
// trivially clear; null entries are skipped (defensive); ineligible
// unsatisfied entries don't gate; one eligible-but-unsatisfied entry
// blocks the clear.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsLevelClearedEmptyTest,
	"Quake.Stats.LevelCleared.EmptySet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsLevelClearedEmptyTest::RunTest(const FString&)
{
	TArray<const AQuakeEnemySpawnPoint*> Empty;
	TestTrue(TEXT("Empty set is cleared"), AQuakeGameMode::IsLevelClearedForSet(Empty));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsLevelClearedIgnoresNullTest,
	"Quake.Stats.LevelCleared.IgnoresNullEntries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsLevelClearedIgnoresNullTest::RunTest(const FString&)
{
	TArray<const AQuakeEnemySpawnPoint*> WithNulls = { nullptr, nullptr };
	TestTrue(TEXT("Null-only set treated as cleared"),
		AQuakeGameMode::IsLevelClearedForSet(WithNulls));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsLevelClearedDecorationDoesntGateTest,
	"Quake.Stats.LevelCleared.UnmarkedDoesNotGate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsLevelClearedDecorationDoesntGateTest::RunTest(const FString&)
{
	// One decoration spawn point (unmarked, unspawned, never satisfied) —
	// must not block level-clear because it's ineligible.
	AQuakeEnemySpawnPoint* Decoration = MakeSpawnPoint(/*bMarked*/false, EQuakeDifficulty::Easy, false);
	TArray<const AQuakeEnemySpawnPoint*> Set = { Decoration };
	TestTrue(TEXT("Unmarked unsatisfied does not gate"),
		AQuakeGameMode::IsLevelClearedForSet(Set));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsLevelClearedMarkedUnsatisfiedTest,
	"Quake.Stats.LevelCleared.MarkedUnsatisfiedBlocks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsLevelClearedMarkedUnsatisfiedTest::RunTest(const FString&)
{
	AQuakeEnemySpawnPoint* SP = MakeSpawnPoint(true, EQuakeDifficulty::Easy, false);
	TArray<const AQuakeEnemySpawnPoint*> Set = { SP };
	TestFalse(TEXT("Marked unsatisfied blocks clear"),
		AQuakeGameMode::IsLevelClearedForSet(Set));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsLevelClearedAllSatisfiedTest,
	"Quake.Stats.LevelCleared.AllSatisfied",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsLevelClearedAllSatisfiedTest::RunTest(const FString&)
{
	AQuakeEnemySpawnPoint* A = MakeSpawnPoint(true, EQuakeDifficulty::Easy, false);
	A->SpawnedEnemy = MakeEnemyAtHealth(0.f);
	AQuakeEnemySpawnPoint* B = MakeSpawnPoint(true, EQuakeDifficulty::Easy, true);
	B->SpawnedEnemy = MakeEnemyAtHealth(0.f);
	TArray<const AQuakeEnemySpawnPoint*> Set = { A, B };
	TestTrue(TEXT("Two satisfied points clear the level"),
		AQuakeGameMode::IsLevelClearedForSet(Set));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsLevelClearedDeferredBlocksTest,
	"Quake.Stats.LevelCleared.DeferredUnspawnedBlocks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsLevelClearedDeferredBlocksTest::RunTest(const FString&)
{
	// SPEC 5.9: a deferred spawn point that has never fired is eligible
	// but not satisfied — must keep the level unclear until both happen.
	AQuakeEnemySpawnPoint* Immediate = MakeSpawnPoint(true, EQuakeDifficulty::Easy, false);
	Immediate->SpawnedEnemy = MakeEnemyAtHealth(0.f);
	AQuakeEnemySpawnPoint* Deferred  = MakeSpawnPoint(true, EQuakeDifficulty::Easy, true);
	// Deferred->SpawnedEnemy intentionally null — its trigger never fired.
	TArray<const AQuakeEnemySpawnPoint*> Set = { Immediate, Deferred };
	TestFalse(TEXT("Unfired deferred spawn point blocks level-clear"),
		AQuakeGameMode::IsLevelClearedForSet(Set));
	return true;
}

// -----------------------------------------------------------------------------
// SPEC 5.9 forward-compat: a "Down" zombie reports IsDead()=false until its
// permanent kill. Modeling that here with a Health > 0 enemy on a marked
// spawn point — the spawn point should NOT be satisfied, matching the SPEC
// note that revived zombies don't satisfy their spawn point until they're
// permanently dead. (Zombies aren't in v1; this regresses the predicate.)
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsZombieReviveForwardCompatTest,
	"Quake.Stats.SpawnPoint.Satisfied.RevivedZombieStillUnsatisfied",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsZombieReviveForwardCompatTest::RunTest(const FString&)
{
	AQuakeEnemySpawnPoint* SP = MakeSpawnPoint(true, EQuakeDifficulty::Easy, false);
	AQuakeEnemyBase* Revived = MakeEnemyAtHealth(15.f);  // "Down state" -> alive again
	SP->SpawnedEnemy = Revived;
	TestFalse(TEXT("Revived (alive) zombie keeps spawn point unsatisfied"),
		SP->IsSatisfied());

	Revived->SetHealthForTest(0.f);  // Permanent kill.
	TestTrue(TEXT("Permanent kill satisfies spawn point"), SP->IsSatisfied());
	return true;
}

// -----------------------------------------------------------------------------
// AQuakePlayerState credit + lifecycle methods — SPEC 5.9 / 1.4.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsPlayerStateCreditTest,
	"Quake.Stats.PlayerState.CreditIncrements",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsPlayerStateCreditTest::RunTest(const FString&)
{
	AQuakePlayerState* PS = NewObject<AQuakePlayerState>();
	TestEqual(TEXT("Kills start at 0"),   PS->Kills,   0);
	TestEqual(TEXT("Secrets start at 0"), PS->Secrets, 0);
	TestEqual(TEXT("Deaths start at 0"),  PS->Deaths,  0);

	PS->AddKillCredit();
	PS->AddKillCredit();
	PS->AddSecretCredit();
	PS->AddDeath();

	TestEqual(TEXT("Kills += 2"),   PS->Kills,   2);
	TestEqual(TEXT("Secrets += 1"), PS->Secrets, 1);
	TestEqual(TEXT("Deaths += 1"),  PS->Deaths,  1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeStatsPlayerStateClearPerLifeTest,
	"Quake.Stats.PlayerState.ClearPerLifePreservesScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeStatsPlayerStateClearPerLifeTest::RunTest(const FString&)
{
	// SPEC 1.4 / 6.4: ClearPerLifeState empties powerups but preserves the
	// per-attempt score (Kills / Secrets / Deaths).
	AQuakePlayerState* PS = NewObject<AQuakePlayerState>();
	PS->AddKillCredit();
	PS->AddSecretCredit();
	PS->AddDeath();
	PS->GivePowerup(EQuakePowerup::Quad, 30.f);

	PS->ClearPerLifeState();

	TestEqual(TEXT("Kills survive ClearPerLifeState"),   PS->Kills,   1);
	TestEqual(TEXT("Secrets survive ClearPerLifeState"), PS->Secrets, 1);
	TestEqual(TEXT("Deaths survive ClearPerLifeState"),  PS->Deaths,  1);
	TestEqual(TEXT("ActivePowerups emptied"),            PS->ActivePowerups.Num(), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
