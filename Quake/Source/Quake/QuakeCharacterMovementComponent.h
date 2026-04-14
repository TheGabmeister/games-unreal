#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "QuakeCharacterMovementComponent.generated.h"

/**
 * Custom CharacterMovementComponent implementing Quake-style air acceleration.
 *
 * The critical difference from stock UCharacterMovementComponent is in the
 * airborne case: stock CMC clamps horizontal velocity to MaxWalkSpeed in
 * CalcVelocity (via GetClampedToMaxSize), which fundamentally breaks Quake
 * strafe jumping. This subclass instead applies the original Quake
 * PM_AirAccelerate formula, which clamps the DOT PRODUCT of velocity and
 * wishdir to MaxAirSpeedGain, leaving the velocity magnitude unclamped.
 * That dot-product clamp is what lets the player accumulate horizontal
 * speed indefinitely by air-strafing.
 *
 * See DESIGN.md section 1.1 and CLAUDE.md "Risk Note: Strafe-Jumping CMC".
 */
UCLASS()
class QUAKE_API UQuakeCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UQuakeCharacterMovementComponent();

	/**
	 * Quake air-accel coefficient. Plays the role of 'accel' in the
	 * PM_AirAccelerate formula:
	 *     accelspeed = QuakeAirAcceleration * MaxAirSpeedGain * deltaTime
	 * With defaults (100, 30), this is always larger than MaxAirSpeedGain
	 * at standard frame rates, so the accel always clamps to the remaining
	 * addspeed — which is the intended Quake behavior (per-tick gain of
	 * exactly MaxAirSpeedGain along wishdir when strafing perpendicular).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement (Quake)", meta = (ClampMin = "0.0"))
	float QuakeAirAcceleration = 100.f;

	/**
	 * Maximum horizontal speed the player can gain per air-strafe tick
	 * along the wishdir axis. This is the dot-product clamp — it clamps
	 * the projection of velocity onto wishdir, NOT the velocity magnitude.
	 * Matches Quake's sv_maxairspeed = 30.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement (Quake)", meta = (ClampMin = "0.0"))
	float MaxAirSpeedGain = 30.f;

	/**
	 * Post-landing grace window (seconds) during which a Jump press
	 * preserves the pre-landing horizontal velocity instead of letting
	 * ground friction bleed it off. This is the mechanic that makes
	 * bunny-hopping work: chain jumps within BunnyHopWindow of each
	 * landing and the horizontal speed accumulated in the last air-strafe
	 * cycle carries forward into the next.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement (Quake)", meta = (ClampMin = "0.0"))
	float BunnyHopWindow = 0.1f;

	/**
	 * Pure function implementing the Quake PM_AirAccelerate formula.
	 * Extracted as a static so it can be unit-tested without spinning up
	 * a world, a pawn, or a CMC instance.
	 *
	 * Only the X/Y components of velocity and wishdir are used; Z is
	 * passed through on the returned velocity (gravity is PhysFalling's
	 * job, not ours).
	 *
	 * @param InVelocity     current velocity
	 * @param InWishDir      desired move direction (will be normalized, Z ignored)
	 * @param AccelCoef      per-tick acceleration multiplier (QuakeAirAcceleration)
	 * @param MaxSpeedGain   dot-product clamp (MaxAirSpeedGain)
	 * @param DeltaTime      simulation delta in seconds
	 * @return               new velocity after one tick of air accel
	 */
	static FVector ApplyQuakeAirAccel(
		const FVector& InVelocity,
		const FVector& InWishDir,
		float AccelCoef,
		float MaxSpeedGain,
		float DeltaTime);

	/** True iff within BunnyHopWindow seconds of the last landing. */
	bool IsInBunnyHopWindow() const;

	// --- UCharacterMovementComponent overrides ---

	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual bool DoJump(bool bReplayingMoves, float DeltaTime) override;
	virtual void ProcessLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations) override;

private:
	/** World time (GetTimeSeconds) at the most recent landing. -1 means "no recent landing". */
	double LastLandedWorldTime = -1.0;

	/** Horizontal velocity captured at the instant of landing, preserved for bunny hops. */
	FVector PreLandingHorizontalVelocity = FVector::ZeroVector;
};
