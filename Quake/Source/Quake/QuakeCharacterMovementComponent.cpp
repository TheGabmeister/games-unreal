#include "QuakeCharacterMovementComponent.h"

#include "Engine/World.h"

UQuakeCharacterMovementComponent::UQuakeCharacterMovementComponent()
{
	// Ground movement parameters from SPEC section 1.1.
	MaxWalkSpeed = 600.f;                  // SPEC: Max ground speed
	MaxAcceleration = 6000.f;              // SPEC: Ground acceleration
	GroundFriction = 8.f;                  // SPEC: Ground friction
	BrakingDecelerationWalking = 800.f;    // Feel-tuned under the GroundFriction=8 curve.

	// Jumping.
	JumpZVelocity = 420.f;                 // SPEC: Jump velocity
	AirControl = 0.3f;                     // SPEC: Air control (stock CMC parameter;
	                                       // our Quake air accel is what actually
	                                       // drives horizontal movement while airborne).
	GravityScale = 1.f;                    // UE default world gravity is 980 u/s^2 which matches SPEC.

	// Slopes and steps.
	SetWalkableFloorAngle(44.f);           // SPEC: 44 deg walkable; steeper = wall
	MaxStepHeight = 45.f;                  // SPEC: 45u step height

	// No crouch and no fall damage are the "do nothing" case — we don't
	// enable them anywhere else, so no explicit disable is needed.
}

FVector UQuakeCharacterMovementComponent::ApplyQuakeAirAccel(
	const FVector& InVelocity,
	const FVector& InWishDir,
	float AccelCoef,
	float MaxSpeedGain,
	float DeltaTime)
{
	if (MaxSpeedGain <= 0.f || DeltaTime <= 0.f)
	{
		return InVelocity;
	}

	// Work on the horizontal plane only — Z is gravity's job, not ours.
	const FVector WishDir2D = FVector(InWishDir.X, InWishDir.Y, 0.f).GetSafeNormal();
	if (WishDir2D.IsNearlyZero())
	{
		return InVelocity;
	}

	// Quake PM_AirAccelerate (quakeworld/client/pmove.c):
	//     currentspeed = DotProduct(velocity, wishdir)   <-- THE key: dot, not magnitude
	//     addspeed     = wishspeed - currentspeed
	//     if (addspeed <= 0) return
	//     accelspeed   = accel * wishspeed * frametime
	//     if (accelspeed > addspeed) accelspeed = addspeed
	//     velocity    += accelspeed * wishdir
	//
	// Here "wishspeed" is MaxSpeedGain (Quake's sv_maxairspeed = 30), NOT MaxWalkSpeed.
	// The dot-product clamp is what lets the player accumulate horizontal speed
	// well beyond MaxWalkSpeed by strafing perpendicular to their velocity:
	// when velocity and wishdir are orthogonal, currentspeed=0, so each tick
	// adds MaxSpeedGain along a brand-new axis without ever touching the
	// existing forward speed.
	const FVector HorizontalVel(InVelocity.X, InVelocity.Y, 0.f);
	const float CurrentSpeed = FVector::DotProduct(HorizontalVel, WishDir2D);
	const float AddSpeed = MaxSpeedGain - CurrentSpeed;
	if (AddSpeed <= 0.f)
	{
		return InVelocity;
	}

	float AccelSpeed = AccelCoef * MaxSpeedGain * DeltaTime;
	if (AccelSpeed > AddSpeed)
	{
		AccelSpeed = AddSpeed;
	}

	return FVector(
		InVelocity.X + AccelSpeed * WishDir2D.X,
		InVelocity.Y + AccelSpeed * WishDir2D.Y,
		InVelocity.Z);
}

void UQuakeCharacterMovementComponent::CalcVelocity(
	float DeltaTime,
	float Friction,
	bool bFluid,
	float BrakingDeceleration)
{
	if (MovementMode != MOVE_Falling)
	{
		Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
		return;
	}

	// Airborne: do NOT call Super. Stock UCharacterMovementComponent::CalcVelocity
	// ends with `Velocity = Velocity.GetClampedToMaxSize(NewMaxInputSpeed)` where
	// NewMaxInputSpeed is MaxWalkSpeed during falling — that magnitude clamp is
	// exactly what breaks Quake strafe jumping. Instead we apply the
	// dot-product-clamped air accel directly.
	//
	// Note: PhysFalling has already zeroed Velocity.Z before calling us and will
	// restore it afterward (see UE CharacterMovementComponent.cpp PhysFalling,
	// around the CalcVelocity call). So we only need to touch horizontal
	// components here. ApplyQuakeAirAccel preserves Velocity.Z.
	const FVector WishDir = Acceleration.GetSafeNormal();
	if (WishDir.IsNearlyZero())
	{
		// No input — no air control, no friction. Quake has no air friction on
		// the player; the airborne glide continues unchanged until landing.
		return;
	}

	Velocity = ApplyQuakeAirAccel(Velocity, WishDir, QuakeAirAcceleration, MaxAirSpeedGain, DeltaTime);
}

bool UQuakeCharacterMovementComponent::DoJump(bool bReplayingMoves, float DeltaTime)
{
	// Capture eligibility BEFORE the jump executes, because Super::DoJump will
	// change state (MovementMode, Velocity.Z) that could confuse the check.
	const bool bWasBunnyHopEligible = IsInBunnyHopWindow();
	const FVector BunnyHopVelocity = PreLandingHorizontalVelocity;

	const bool bJumped = Super::DoJump(bReplayingMoves, DeltaTime);

	if (bJumped && bWasBunnyHopEligible)
	{
		// Restore pre-landing horizontal velocity. Super::DoJump only sets
		// Velocity.Z = JumpZVelocity; it does not touch X/Y. Overwriting
		// here puts us back at the horizontal speed we had at the moment
		// of the previous landing, before ground friction bled it off.
		Velocity.X = BunnyHopVelocity.X;
		Velocity.Y = BunnyHopVelocity.Y;

		// Consume the window so a second jump in the same airborne period
		// (which would have LastLandedWorldTime still set) does not re-apply
		// the preserved velocity.
		LastLandedWorldTime = -1.0;
	}

	return bJumped;
}

void UQuakeCharacterMovementComponent::ProcessLanded(
	const FHitResult& Hit,
	float RemainingTime,
	int32 Iterations)
{
	// Capture horizontal velocity at the instant of landing, before UE's
	// ground friction starts decaying it. DoJump consumes this within
	// BunnyHopWindow seconds.
	PreLandingHorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.f);
	if (const UWorld* World = GetWorld())
	{
		LastLandedWorldTime = World->GetTimeSeconds();
	}

	Super::ProcessLanded(Hit, RemainingTime, Iterations);
}

bool UQuakeCharacterMovementComponent::IsInBunnyHopWindow() const
{
	if (LastLandedWorldTime < 0.0 || BunnyHopWindow <= 0.f)
	{
		return false;
	}
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	return (World->GetTimeSeconds() - LastLandedWorldTime) <= static_cast<double>(BunnyHopWindow);
}
