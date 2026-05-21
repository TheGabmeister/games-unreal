#include "TH2MovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

UTH2MovementComponent::UTH2MovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTH2MovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent) return;

	if (bIsGrounded)
	{
		UpdateGroundMovement(DeltaTime);
	}
	else
	{
		UpdateAirMovement(DeltaTime);
	}

	// Sweep move
	FHitResult MoveHit;
	const FVector Delta = SkateVelocity * DeltaTime;
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, MoveHit);

	if (MoveHit.bBlockingHit)
	{
		// Check wall vs ramp
		const float WallThreshold = 0.3f; // dot with up vector — below this is a wall
		if (FVector::DotProduct(MoveHit.ImpactNormal, FVector::UpVector) < WallThreshold)
		{
			HandleWallHit(MoveHit);
		}
		else
		{
			SlideAlongSurface(Delta, 1.f - MoveHit.Time, MoveHit.Normal, MoveHit);
		}
	}

	// Ground check for airborne pawn
	if (!bIsGrounded)
	{
		FHitResult GroundHit;
		if (SkateVelocity.Z <= 0.f && TraceGround(GroundHit))
		{
			HandleLanding(GroundHit);
		}
	}

	UpdateComponentVelocity();
}

void UTH2MovementComponent::UpdateGroundMovement(float DeltaTime)
{
	const FVector Forward = PawnOwner->GetActorForwardVector();

	// Auto-kick: gradual speed decay
	if (bAutoKick)
	{
		float CurrentSpeed = FVector::DotProduct(SkateVelocity, Forward);
		if (CurrentSpeed > 0.f)
		{
			CurrentSpeed *= (1.f - SpeedDecayRate);
			if (CurrentSpeed < 10.f) CurrentSpeed = 0.f;
		}

		// Auto push acceleration when moving forward
		if (CurrentSpeed < MaxSpeed * 0.3f && !bIsBraking && CurrentSpeed >= 0.f)
		{
			CurrentSpeed += AutoKickAcceleration * DeltaTime;
		}

		SkateVelocity = Forward * FMath::Min(CurrentSpeed, MaxSpeed);
	}

	// Braking
	if (bIsBraking)
	{
		ApplyBraking(DeltaTime);
	}

	// Gravity along slope (push downhill, resist uphill)
	const float SlopeGravity = FVector::DotProduct(FVector(0, 0, -980.f), Forward);
	SkateVelocity += Forward * SlopeGravity * DeltaTime;

	// Clamp speed
	float Speed = SkateVelocity.Size();
	if (Speed > MaxSpeed)
	{
		SkateVelocity = SkateVelocity.GetSafeNormal() * MaxSpeed;
	}

	// Zero out vertical component on ground (velocity is along surface)
	SkateVelocity.Z = 0.f;
	if (GroundNormal != FVector::UpVector)
	{
		// Project velocity onto the ground plane
		SkateVelocity = FVector::VectorPlaneProject(SkateVelocity, GroundNormal);
	}

	// Surface alignment
	FHitResult GroundHit;
	if (TraceGround(GroundHit))
	{
		GroundNormal = GroundHit.ImpactNormal;
		AlignToSurface(GroundNormal, DeltaTime);

		// Detect surface type
		float UpDot = FVector::DotProduct(GroundNormal, FVector::UpVector);
		if (UpDot > 0.95f)
			CurrentSurface = ETH2SurfaceType::Flat;
		else if (UpDot > 0.3f)
			CurrentSurface = ETH2SurfaceType::Ramp;
		else
			CurrentSurface = ETH2SurfaceType::Vert;

		// Check if we're leaving a ramp (vert launch)
		if (CurrentSurface == ETH2SurfaceType::Vert && Speed > 100.f)
		{
			FVector LaunchDir = GroundNormal;
			LaunchDir.Z = FMath::Max(LaunchDir.Z, 0.7f);
			LaunchDir.Normalize();
			LaunchFromRamp(LaunchDir, Speed);
			return;
		}
	}
	else
	{
		// Lost ground contact — become airborne
		bIsGrounded = false;
		AirTime = 0.f;
		GroundNormal = FVector::UpVector;
	}
}

void UTH2MovementComponent::UpdateAirMovement(float DeltaTime)
{
	AirTime += DeltaTime;

	// Gravity with hangtime
	float EffectiveGravity = -980.f * GravityScale;
	if (FMath::Abs(SkateVelocity.Z) < HangtimeVelocityThreshold)
	{
		EffectiveGravity *= HangtimeGravityScale;
	}

	SkateVelocity.Z += EffectiveGravity * DeltaTime;
}

bool UTH2MovementComponent::TraceGround(FHitResult& OutHit) const
{
	if (!UpdatedComponent) return false;

	const FVector Start = UpdatedComponent->GetComponentLocation();
	const FVector End = Start + FVector(0, 0, -GroundTraceDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PawnOwner);

	return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}

void UTH2MovementComponent::AlignToSurface(const FVector& Normal, float DeltaTime)
{
	if (!PawnOwner) return;

	const FVector Forward = PawnOwner->GetActorForwardVector();
	const FVector Right = FVector::CrossProduct(Normal, Forward).GetSafeNormal();
	const FVector AlignedForward = FVector::CrossProduct(Right, Normal).GetSafeNormal();

	FRotator Current = PawnOwner->GetActorRotation();
	FRotator Target = UKismetMathLibrary::MakeRotFromXZ(AlignedForward, Normal);
	Target.Yaw = Current.Yaw; // preserve steering yaw

	FRotator NewRot = FMath::RInterpTo(Current, Target, DeltaTime, SurfaceAlignSpeed);
	PawnOwner->SetActorRotation(NewRot);
}

void UTH2MovementComponent::ApplySteeringInput(float SteerValue, float DeltaTime)
{
	if (!PawnOwner || FMath::IsNearlyZero(SteerValue)) return;

	float Speed = SkateVelocity.Size();
	float SpeedRatio = FMath::Clamp(Speed / MaxSpeed, 0.f, 1.f);
	// Sharper turns at low speed, wider at high speed
	float TurnRate = FMath::Lerp(TurnRateMax, TurnRateMin, SpeedRatio);

	FRotator Rot = PawnOwner->GetActorRotation();
	Rot.Yaw += SteerValue * TurnRate * DeltaTime;
	PawnOwner->SetActorRotation(Rot);

	// Re-align velocity to new forward direction
	if (bIsGrounded && Speed > 0.f)
	{
		SkateVelocity = PawnOwner->GetActorForwardVector() * Speed;
	}
}

void UTH2MovementComponent::ApplyBraking(float DeltaTime)
{
	float Speed = FVector::DotProduct(SkateVelocity, PawnOwner->GetActorForwardVector());
	Speed -= BrakeDeceleration * DeltaTime;
	if (Speed < 0.f) Speed = 0.f;
	SkateVelocity = PawnOwner->GetActorForwardVector() * Speed;
}

void UTH2MovementComponent::LaunchOllie(float ChargeRatio, float Multiplier)
{
	if (!bIsGrounded) return;

	float Impulse = FMath::Lerp(MinOllieImpulse, MaxOllieImpulse, ChargeRatio) * Multiplier;
	SkateVelocity.Z = Impulse;
	bIsGrounded = false;
	AirTime = 0.f;
}

void UTH2MovementComponent::LaunchFromRamp(const FVector& LaunchDirection, float Speed)
{
	SkateVelocity = LaunchDirection * Speed;
	bIsGrounded = false;
	AirTime = 0.f;
}

void UTH2MovementComponent::HandleLanding(const FHitResult& Hit)
{
	bIsGrounded = true;
	GroundNormal = Hit.ImpactNormal;

	// Project velocity onto ground plane
	SkateVelocity = FVector::VectorPlaneProject(SkateVelocity, GroundNormal);

	// Snap to ground
	if (UpdatedComponent)
	{
		FVector Loc = UpdatedComponent->GetComponentLocation();
		Loc.Z = Hit.ImpactPoint.Z + GroundTraceDistance * 0.5f;
		UpdatedComponent->SetWorldLocation(Loc);
	}

	AlignToSurface(GroundNormal, 1.f);
}

void UTH2MovementComponent::HandleWallHit(const FHitResult& Hit)
{
	float Speed = SkateVelocity.Size();

	if (Speed > WallBailSpeedThreshold)
	{
		// High-speed wall hit — bail handled by pawn
		SkateVelocity = FVector::ZeroVector;
		// Pawn checks for wall bail via OnWallHit delegate or polling
	}
	else
	{
		// Slide along wall
		SkateVelocity = FVector::VectorPlaneProject(SkateVelocity, Hit.ImpactNormal);
	}
}
