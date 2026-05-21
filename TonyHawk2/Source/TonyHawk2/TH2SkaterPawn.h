#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TH2SkaterPawn.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UAudioComponent;
class USoundWave;
class UTH2MovementComponent;

UENUM(BlueprintType)
enum class ETH2SkaterState : uint8
{
	Idle,
	Pushing,
	Crouching,
	Airborne,
	Landing,
	Bailing
};

UENUM(BlueprintType)
enum class ETH2LaunchType : uint8
{
	Normal,
	Nollie,
	NoComply,
	Boneless
};

UCLASS()
class TONYHAWK2_API ATH2SkaterPawn : public APawn
{
	GENERATED_BODY()

public:
	ATH2SkaterPawn();

	virtual void Tick(float DeltaTime) override;

	// --- Components ---
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCapsuleComponent> CapsuleComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BoardMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTH2MovementComponent> MovementComp;

	// --- Audio ---
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAudioComponent> WheelRollAudio;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAudioComponent> WindAirAudio;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundWave> OlliePopSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundWave> LandingThudSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundWave> BailImpactSound;

	// --- Input interface (called by PlayerController) ---
	void SetMoveInput(const FVector2D& Value);
	void StartOllie();
	void ReleaseOllie();
	void SetBraking(bool bBrake);
	void ToggleSwitchStance();

	// --- State ---
	UPROPERTY(BlueprintReadOnly)
	ETH2SkaterState CurrentState = ETH2SkaterState::Idle;

	UPROPERTY(BlueprintReadOnly)
	bool bSwitchStance = false;

	UPROPERTY(BlueprintReadOnly)
	ETH2LaunchType LastLaunchType = ETH2LaunchType::Normal;

	// --- Camera ---
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float DefaultArmLength = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float VertArmLength = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraLagSpeed = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float VertLaunchAngleThreshold = 60.f;

	// --- Colors ---
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FLinearColor IdleColor = FLinearColor(0.5f, 0.5f, 0.5f);

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FLinearColor PushingColor = FLinearColor(0.1f, 0.8f, 0.2f);

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FLinearColor CrouchingColor = FLinearColor(0.9f, 0.8f, 0.1f);

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FLinearColor AirborneColor = FLinearColor(0.2f, 0.4f, 0.9f);

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FLinearColor LandingColor = FLinearColor(1.f, 1.f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FLinearColor BailColor = FLinearColor(0.9f, 0.1f, 0.1f);

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FLinearColor SwitchTintColor = FLinearColor(1.f, 0.6f, 0.2f);

private:
	virtual UPawnMovementComponent* GetMovementComponent() const override;

	void UpdateState(float DeltaTime);
	void SetState(ETH2SkaterState NewState);
	void UpdateCapsuleColor();
	void UpdateCamera(float DeltaTime);
	void UpdateAudio();
	void CheckAdvancedLaunch();

	// Ollie charge
	bool bOllieHeld = false;
	float OllieChargeTime = 0.f;
	float MaxOllieChargeTime = 0.5f;

	// Bail timer
	float BailTimer = 0.f;
	float BailDuration = 1.5f;

	// Landing flash
	float LandingFlashTimer = 0.f;
	float LandingFlashDuration = 0.3f;

	// Big Drop
	bool bBigDropWindowActive = false;
	bool bBigDropPressed = false;

	// Input buffer for advanced launches
	struct FInputBufferEntry
	{
		FVector2D Direction;
		double Timestamp;
	};
	TArray<FInputBufferEntry> InputBuffer;
	float InputBufferWindow = 0.27f; // ~16 frames at 60fps

	// Move input
	FVector2D CurrentMoveInput = FVector2D::ZeroVector;

	// Dynamic material
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;
};
