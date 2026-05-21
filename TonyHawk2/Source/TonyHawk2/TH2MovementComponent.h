#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "TH2MovementComponent.generated.h"

UENUM(BlueprintType)
enum class ETH2SurfaceType : uint8
{
	Flat,
	Ramp,
	Vert,
	Wall
};

UCLASS()
class TONYHAWK2_API UTH2MovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	UTH2MovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Movement parameters (hardcoded defaults, stat-driven in Phase 7) ---
	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float MaxSpeed = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float AutoKickAcceleration = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float SpeedDecayRate = 0.005f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float BrakeDeceleration = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float TurnRateMin = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float TurnRateMax = 160.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float MinOllieImpulse = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float MaxOllieImpulse = 750.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float BonelessMultiplier = 1.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float GravityScale = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float HangtimeGravityScale = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float HangtimeVelocityThreshold = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float GroundTraceDistance = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float BigDropVelocityThreshold = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float WallBailSpeedThreshold = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skating")
	float SurfaceAlignSpeed = 10.f;

	// --- State ---
	FVector SkateVelocity = FVector::ZeroVector;
	bool bIsGrounded = true;
	bool bIsBraking = false;
	bool bAutoKick = true;
	ETH2SurfaceType CurrentSurface = ETH2SurfaceType::Flat;
	FVector GroundNormal = FVector::UpVector;
	float AirTime = 0.f;

	// --- Interface ---
	void ApplySteeringInput(float SteerValue, float DeltaTime);
	void ApplyBraking(float DeltaTime);
	void LaunchOllie(float ChargeRatio, float Multiplier = 1.f);
	void LaunchFromRamp(const FVector& LaunchDirection, float Speed);

	bool IsGrounded() const { return bIsGrounded; }
	float GetCurrentSpeed() const { return SkateVelocity.Size(); }
	float GetVerticalVelocity() const { return SkateVelocity.Z; }
	bool IsBigDrop() const { return !bIsGrounded && SkateVelocity.Z < -BigDropVelocityThreshold; }

private:
	void UpdateGroundMovement(float DeltaTime);
	void UpdateAirMovement(float DeltaTime);
	bool TraceGround(FHitResult& OutHit) const;
	void AlignToSurface(const FVector& Normal, float DeltaTime);
	void HandleLanding(const FHitResult& Hit);
	void HandleWallHit(const FHitResult& Hit);
};
