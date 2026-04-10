// Unit tests for UQuakeCharacterMovementComponent's air-acceleration formula.
//
// These cover the SPEC Phase 1 automated-test requirements that can be
// exercised as pure logic (no world, no pawn, no physics tick). The three
// "functional" tests the spec also lists (FT_StrafeJump, bunny-hop sequence,
// walkable slope) require an in-editor test map and are authored separately
// under Content/Maps/Tests/ per SPEC section 11.1.
//
// Run from Session Frontend -> Automation tab with filter "Quake.*".
//
// All numeric literals are doubles — UE 5.5+ uses Large World Coordinates,
// so FVector components are doubles. Mixing float and double in TestEqual
// triggers an ambiguous-overload compile error.

#include "Misc/AutomationTest.h"
#include "QuakeCharacterMovementComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr double kQuakeAirAccel     = 100.0;
	constexpr double kMaxAirSpeedGain   = 30.0;
	constexpr double kTickDeltaTime     = 1.0 / 60.0;
	constexpr double kTolerance         = UE_KINDA_SMALL_NUMBER;
}

// -----------------------------------------------------------------------------
// The canonical SPEC unit test:
//     Velocity = (300, 0, 0), WishDir = (0, 1, 0), MaxAirSpeedGain = 30
//     Assert post-tick velocity gains exactly 30 in the wish direction
//     (NOT capped to MaxGroundSpeed).
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAirAccelDotProductClampTest,
	"Quake.Movement.AirAccel.DotProductClamp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAirAccelDotProductClampTest::RunTest(const FString& /*Parameters*/)
{
	const FVector StartVel(300.0, 0.0, 0.0);
	const FVector WishDir(0.0, 1.0, 0.0);

	const FVector NewVel = UQuakeCharacterMovementComponent::ApplyQuakeAirAccel(
		StartVel, WishDir, kQuakeAirAccel, kMaxAirSpeedGain, kTickDeltaTime);

	TestEqual(TEXT("X velocity preserved (no magnitude clamp)"),
		NewVel.X, 300.0, kTolerance);
	TestEqual(TEXT("Y velocity gained exactly MaxAirSpeedGain"),
		NewVel.Y, 30.0, kTolerance);
	TestEqual(TEXT("Z velocity untouched by horizontal accel"),
		NewVel.Z, 0.0, kTolerance);

	// Sanity: the resulting magnitude is sqrt(300^2 + 30^2) ≈ 301.5, which
	// exceeds MaxAirSpeedGain (30) but not MaxWalkSpeed (600). The point of
	// this assertion is to prove that NO magnitude clamp was applied — under
	// stock UE CMC, Y would be pinned such that |v| never exceeds
	// MaxWalkSpeed, which is exactly the bug Quake strafe-jumping requires
	// us to fix.
	const double NewSpeed = NewVel.Size();
	TestTrue(TEXT("New speed reflects unclamped horizontal gain"),
		NewSpeed > 300.0 && NewSpeed < 400.0);

	return true;
}

// -----------------------------------------------------------------------------
// When current speed along wishdir is already at MaxAirSpeedGain, AddSpeed
// clamps to zero and no further gain should happen. This is the "cap hit"
// end of the clamp — it's what limits air control in the forward direction
// and is equally part of Quake's air feel.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAirAccelAtMaxWishSpeedTest,
	"Quake.Movement.AirAccel.AtMaxWishSpeed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAirAccelAtMaxWishSpeedTest::RunTest(const FString& /*Parameters*/)
{
	// Velocity aligned with wishdir, already at the 30 u/s cap.
	const FVector StartVel(30.0, 0.0, 0.0);
	const FVector WishDir(1.0, 0.0, 0.0);

	const FVector NewVel = UQuakeCharacterMovementComponent::ApplyQuakeAirAccel(
		StartVel, WishDir, kQuakeAirAccel, kMaxAirSpeedGain, kTickDeltaTime);

	TestEqual(TEXT("No gain when already at wishspeed cap"),
		NewVel.X, 30.0, kTolerance);
	TestEqual(TEXT("Perpendicular axis unaffected"),
		NewVel.Y, 0.0, kTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// The regression test against the stock-CMC clamp-to-MaxWalkSpeed bug.
// If somebody accidentally re-introduces a magnitude clamp, the player's
// horizontal velocity at 1000 u/s would be pinned back down to 600 by a
// single air-strafe tick. Assert that does NOT happen here.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAirAccelHighSpeedStrafeTest,
	"Quake.Movement.AirAccel.HighSpeedStrafe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAirAccelHighSpeedStrafeTest::RunTest(const FString& /*Parameters*/)
{
	const FVector StartVel(1000.0, 0.0, 0.0);  // Well above MaxWalkSpeed.
	const FVector WishDir(0.0, 1.0, 0.0);      // Strafe perpendicular.

	const FVector NewVel = UQuakeCharacterMovementComponent::ApplyQuakeAirAccel(
		StartVel, WishDir, kQuakeAirAccel, kMaxAirSpeedGain, kTickDeltaTime);

	TestEqual(TEXT("X at 1000 u/s is preserved (no magnitude clamp to MaxWalkSpeed)"),
		NewVel.X, 1000.0, kTolerance);
	TestEqual(TEXT("Y gains the full dot-product clamp amount"),
		NewVel.Y, 30.0, kTolerance);

	const double NewSpeed = NewVel.Size();
	TestTrue(TEXT("Resulting speed still exceeds 1000 u/s (> MaxWalkSpeed by far)"),
		NewSpeed > 1000.0);
	return true;
}

// -----------------------------------------------------------------------------
// No input -> no air control. The glide continues unchanged. This also
// covers the bail path in CalcVelocity when Acceleration is zero.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAirAccelNoInputTest,
	"Quake.Movement.AirAccel.NoInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAirAccelNoInputTest::RunTest(const FString& /*Parameters*/)
{
	const FVector StartVel(500.0, 250.0, 0.0);

	const FVector NewVel = UQuakeCharacterMovementComponent::ApplyQuakeAirAccel(
		StartVel, FVector::ZeroVector, kQuakeAirAccel, kMaxAirSpeedGain, kTickDeltaTime);

	TestEqual(TEXT("X unchanged with no input"),
		NewVel.X, StartVel.X, kTolerance);
	TestEqual(TEXT("Y unchanged with no input"),
		NewVel.Y, StartVel.Y, kTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Partial wishdir alignment: velocity and wishdir at 45 degrees.
//   currentspeed = 300 * cos(45) ~= 212.13
//   addspeed     = max(0, 30 - 212.13) = 0
//   So: no gain.
// This catches the "only clamp on perpendicular" error mode, where a naive
// implementation might still add speed because the dot product is low but
// still above the 30 cap.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAirAccelPartialAlignmentTest,
	"Quake.Movement.AirAccel.PartialAlignment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAirAccelPartialAlignmentTest::RunTest(const FString& /*Parameters*/)
{
	const FVector StartVel(300.0, 0.0, 0.0);
	const FVector WishDir = FVector(1.0, 1.0, 0.0).GetSafeNormal();

	const FVector NewVel = UQuakeCharacterMovementComponent::ApplyQuakeAirAccel(
		StartVel, WishDir, kQuakeAirAccel, kMaxAirSpeedGain, kTickDeltaTime);

	// Current speed along wishdir is 300 * cos(45) ~= 212, already way above
	// MaxAirSpeedGain (30), so addspeed is negative -> no change at all.
	TestEqual(TEXT("No change when dot product already exceeds MaxAirSpeedGain"),
		NewVel.X, StartVel.X, kTolerance);
	TestEqual(TEXT("Y also unchanged (bail before accumulating)"),
		NewVel.Y, StartVel.Y, kTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Below the cap along wishdir: expect partial gain (not the full 30).
//   velocity = (10, 0, 0), wishdir = (1, 0, 0), MaxAirSpeedGain = 30
//   currentspeed = 10
//   addspeed     = 20
//   accelspeed   = 100 * 30 * (1/60) = 50  -> clamped to 20
//   newvel       = (30, 0, 0)
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAirAccelPartialGainTest,
	"Quake.Movement.AirAccel.PartialGain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAirAccelPartialGainTest::RunTest(const FString& /*Parameters*/)
{
	const FVector StartVel(10.0, 0.0, 0.0);
	const FVector WishDir(1.0, 0.0, 0.0);

	const FVector NewVel = UQuakeCharacterMovementComponent::ApplyQuakeAirAccel(
		StartVel, WishDir, kQuakeAirAccel, kMaxAirSpeedGain, kTickDeltaTime);

	TestEqual(TEXT("X climbs to exactly MaxAirSpeedGain"),
		NewVel.X, 30.0, kTolerance);
	TestEqual(TEXT("Y untouched"),
		NewVel.Y, 0.0, kTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Z velocity is always preserved by the formula — it's gravity's responsibility,
// not ours. Ensure a falling player's Z component survives an air-strafe tick.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAirAccelPreservesZTest,
	"Quake.Movement.AirAccel.PreservesZ",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAirAccelPreservesZTest::RunTest(const FString& /*Parameters*/)
{
	const FVector StartVel(300.0, 0.0, -400.0);  // Falling while moving forward.
	const FVector WishDir(0.0, 1.0, 0.0);

	const FVector NewVel = UQuakeCharacterMovementComponent::ApplyQuakeAirAccel(
		StartVel, WishDir, kQuakeAirAccel, kMaxAirSpeedGain, kTickDeltaTime);

	TestEqual(TEXT("Z velocity untouched during air strafe"),
		NewVel.Z, -400.0, kTolerance);
	TestEqual(TEXT("Horizontal gain applied normally"),
		NewVel.Y, 30.0, kTolerance);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
