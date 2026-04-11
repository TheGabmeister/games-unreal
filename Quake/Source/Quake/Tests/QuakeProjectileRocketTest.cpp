// Unit tests for the Phase 5 rocket splash falloff formula.
//
// The formula is extracted as a pure static helper
// AQuakeProjectile_Rocket::ComputeLinearFalloffDamage so it can be exercised
// without spinning up a world — same pattern as
// AQuakeCharacter::ApplyArmorAbsorption,
// UQuakeCharacterMovementComponent::ApplyQuakeAirAccel, and
// AQuakeEnemyBase::ComputePainChance.
//
// SPEC section 11.5 Phase 5 test cases:
//   Distance 0   -> full damage
//   Distance 60  -> half damage (half radius with linear falloff)
//   Distance 120 -> 0 damage (at radius edge)
//   Distance 200 -> 0 damage (beyond radius)
//
// Per CLAUDE.md "Running Tests": tests are EditorContext + EngineFilter,
// guarded by WITH_DEV_AUTOMATION_TESTS, and live under Source/Quake/Tests.
// Run from Session Frontend -> Automation tab with filter "Quake.*".

#include "Misc/AutomationTest.h"

#include "QuakeProjectile_Rocket.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr float kFalloffTolerance = UE_KINDA_SMALL_NUMBER;

	// Matches SPEC section 2.0 Rocket Launcher + 2.2 splash radius.
	constexpr float kRocketBaseDamage = 100.f;
	constexpr float kRocketRadius     = 120.f;
}

// -----------------------------------------------------------------------------
// Distance 0 — victim is at the explosion center. Linear falloff at d=0
// returns BaseDamage unchanged. This is the "direct hit" case.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeRocketSplashCenterTest,
	"Quake.Weapon.RocketSplash.Center",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeRocketSplashCenterTest::RunTest(const FString& /*Parameters*/)
{
	const float Damage = AQuakeProjectile_Rocket::ComputeLinearFalloffDamage(
		kRocketBaseDamage, /*Distance*/ 0.f, kRocketRadius);

	TestEqual(TEXT("Direct hit = full BaseDamage"), Damage, 100.f, kFalloffTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Distance 60 — half the radius. Linear formula: 100 * (1 - 60/120) = 50.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeRocketSplashHalfRadiusTest,
	"Quake.Weapon.RocketSplash.HalfRadius",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeRocketSplashHalfRadiusTest::RunTest(const FString& /*Parameters*/)
{
	const float Damage = AQuakeProjectile_Rocket::ComputeLinearFalloffDamage(
		kRocketBaseDamage, /*Distance*/ 60.f, kRocketRadius);

	TestEqual(TEXT("Half radius = half damage"), Damage, 50.f, kFalloffTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Distance 120 — exactly at the radius edge. Falloff is 0 — one damage-unit
// away from the edge should still round down to zero to avoid a one-unit
// nibble at the boundary.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeRocketSplashRadiusEdgeTest,
	"Quake.Weapon.RocketSplash.RadiusEdge",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeRocketSplashRadiusEdgeTest::RunTest(const FString& /*Parameters*/)
{
	const float Damage = AQuakeProjectile_Rocket::ComputeLinearFalloffDamage(
		kRocketBaseDamage, /*Distance*/ 120.f, kRocketRadius);

	TestEqual(TEXT("At the radius edge = 0 damage"), Damage, 0.f, kFalloffTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Distance 200 — beyond the radius. Out-of-range victims take no damage;
// ApplyRadialDamageWithFalloff uses ECC_Visibility for the LoS check and
// otherwise clamps to 0 past the outer radius.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeRocketSplashBeyondRadiusTest,
	"Quake.Weapon.RocketSplash.BeyondRadius",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeRocketSplashBeyondRadiusTest::RunTest(const FString& /*Parameters*/)
{
	const float Damage = AQuakeProjectile_Rocket::ComputeLinearFalloffDamage(
		kRocketBaseDamage, /*Distance*/ 200.f, kRocketRadius);

	TestEqual(TEXT("Beyond radius = 0 damage"), Damage, 0.f, kFalloffTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Degenerate radius guard — a zero-radius rocket (pathological authoring
// bug) should produce zero damage, not a divide-by-zero NaN. This protects
// against a future BP override that accidentally nukes SplashRadius to 0.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeRocketSplashZeroRadiusTest,
	"Quake.Weapon.RocketSplash.ZeroRadius",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeRocketSplashZeroRadiusTest::RunTest(const FString& /*Parameters*/)
{
	const float Damage = AQuakeProjectile_Rocket::ComputeLinearFalloffDamage(
		kRocketBaseDamage, /*Distance*/ 0.f, /*Radius*/ 0.f);

	TestEqual(TEXT("Zero radius = 0 damage (no NaN)"), Damage, 0.f, kFalloffTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Intermediate distance — 30 u with a 120 u radius should yield
// 100 * (1 - 30/120) = 75. This guards the linear interpolation between
// center and edge in case someone accidentally introduces an exponential
// falloff or an inner-radius plateau.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeRocketSplashQuarterRadiusTest,
	"Quake.Weapon.RocketSplash.QuarterRadius",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeRocketSplashQuarterRadiusTest::RunTest(const FString& /*Parameters*/)
{
	const float Damage = AQuakeProjectile_Rocket::ComputeLinearFalloffDamage(
		kRocketBaseDamage, /*Distance*/ 30.f, kRocketRadius);

	TestEqual(TEXT("Quarter radius = three-quarters damage"), Damage, 75.f, kFalloffTolerance);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
