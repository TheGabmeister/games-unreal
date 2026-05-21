#include "TH2SkaterPawn.h"
#include "TH2MovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"

ATH2SkaterPawn::ATH2SkaterPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule collision (root)
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComp->InitCapsuleSize(34.f, 88.f);
	CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));
	SetRootComponent(CapsuleComp);

	// Visual body mesh (placeholder capsule shape — uses engine basic shape)
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(CapsuleComp);
	BodyMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.5f));
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Skateboard mesh
	BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
	BoardMesh->SetupAttachment(CapsuleComp);
	BoardMesh->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	BoardMesh->SetRelativeScale3D(FVector(0.8f, 0.3f, 0.05f));
	BoardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spring arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CapsuleComp);
	SpringArm->TargetArmLength = DefaultArmLength;
	SpringArm->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = CameraLagSpeed;
	SpringArm->bDoCollisionTest = true;

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// Movement component
	MovementComp = CreateDefaultSubobject<UTH2MovementComponent>(TEXT("MovementComp"));
	MovementComp->UpdatedComponent = CapsuleComp;

	// Audio — looping sounds
	WheelRollAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("WheelRollAudio"));
	WheelRollAudio->SetupAttachment(CapsuleComp);
	WheelRollAudio->bAutoActivate = false;

	WindAirAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("WindAirAudio"));
	WindAirAudio->SetupAttachment(CapsuleComp);
	WindAirAudio->bAutoActivate = false;
}

void ATH2SkaterPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Steering from move input
	if (!FMath::IsNearlyZero(CurrentMoveInput.X))
	{
		MovementComp->ApplySteeringInput(CurrentMoveInput.X, DeltaTime);
	}

	// Ollie charge
	if (bOllieHeld && CurrentState == ETH2SkaterState::Crouching)
	{
		OllieChargeTime = FMath::Min(OllieChargeTime + DeltaTime, MaxOllieChargeTime);
	}

	UpdateState(DeltaTime);
	UpdateCapsuleColor();
	UpdateCamera(DeltaTime);
	UpdateAudio();

	// Clear frame input
	CurrentMoveInput = FVector2D::ZeroVector;
}

UPawnMovementComponent* ATH2SkaterPawn::GetMovementComponent() const
{
	return MovementComp;
}

// --- Input interface ---

void ATH2SkaterPawn::SetMoveInput(const FVector2D& Value)
{
	CurrentMoveInput = Value;

	// Buffer directional input for advanced launch detection
	if (!FMath::IsNearlyZero(Value.Y) && Value.Y > 0.5f) // Up input
	{
		FInputBufferEntry Entry;
		Entry.Direction = Value;
		Entry.Timestamp = GetWorld()->GetTimeSeconds();
		InputBuffer.Add(Entry);

		// Prune old entries
		double CutoffTime = Entry.Timestamp - InputBufferWindow;
		InputBuffer.RemoveAll([CutoffTime](const FInputBufferEntry& E) { return E.Timestamp < CutoffTime; });
	}
}

void ATH2SkaterPawn::StartOllie()
{
	if (CurrentState == ETH2SkaterState::Bailing) return;

	if (MovementComp->IsGrounded())
	{
		bOllieHeld = true;
		OllieChargeTime = 0.f;
		SetState(ETH2SkaterState::Crouching);
	}
	else if (MovementComp->IsBigDrop())
	{
		// Big Drop recovery
		bBigDropPressed = true;
	}
}

void ATH2SkaterPawn::ReleaseOllie()
{
	if (!bOllieHeld) return;
	bOllieHeld = false;

	if (CurrentState != ETH2SkaterState::Crouching) return;

	float ChargeRatio = FMath::Clamp(OllieChargeTime / MaxOllieChargeTime, 0.f, 1.f);

	// Check for advanced launch types
	CheckAdvancedLaunch();

	float LaunchMultiplier = 1.f;
	if (LastLaunchType == ETH2LaunchType::Boneless)
	{
		LaunchMultiplier = MovementComp->BonelessMultiplier;
	}

	MovementComp->LaunchOllie(ChargeRatio, LaunchMultiplier);
	SetState(ETH2SkaterState::Airborne);

	// Ollie pop SFX
	if (OlliePopSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, OlliePopSound, GetActorLocation());
	}
}

void ATH2SkaterPawn::SetBraking(bool bBrake)
{
	MovementComp->bIsBraking = bBrake;
}

void ATH2SkaterPawn::ToggleSwitchStance()
{
	if (CurrentState == ETH2SkaterState::Bailing) return;
	bSwitchStance = !bSwitchStance;
}

// --- State management ---

void ATH2SkaterPawn::UpdateState(float DeltaTime)
{
	switch (CurrentState)
	{
	case ETH2SkaterState::Idle:
	case ETH2SkaterState::Pushing:
		if (!MovementComp->IsGrounded())
		{
			SetState(ETH2SkaterState::Airborne);
		}
		else if (MovementComp->GetCurrentSpeed() > 50.f && !MovementComp->bIsBraking)
		{
			if (CurrentState != ETH2SkaterState::Pushing)
				SetState(ETH2SkaterState::Pushing);
		}
		else
		{
			if (CurrentState != ETH2SkaterState::Idle)
				SetState(ETH2SkaterState::Idle);
		}
		break;

	case ETH2SkaterState::Crouching:
		// Handled by ollie input
		break;

	case ETH2SkaterState::Airborne:
		if (MovementComp->IsGrounded())
		{
			// Check for Big Drop bail
			if (bBigDropWindowActive && !bBigDropPressed)
			{
				SetState(ETH2SkaterState::Bailing);
			}
			else
			{
				SetState(ETH2SkaterState::Landing);
				LandingFlashTimer = LandingFlashDuration;

				if (LandingThudSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, LandingThudSound, GetActorLocation());
				}
			}
			bBigDropWindowActive = false;
			bBigDropPressed = false;
		}
		else
		{
			// Check if entering Big Drop territory
			if (MovementComp->IsBigDrop())
			{
				bBigDropWindowActive = true;
			}
		}
		break;

	case ETH2SkaterState::Landing:
		LandingFlashTimer -= DeltaTime;
		if (LandingFlashTimer <= 0.f)
		{
			SetState(ETH2SkaterState::Idle);
		}
		break;

	case ETH2SkaterState::Bailing:
		BailTimer -= DeltaTime;
		if (BailTimer <= 0.f)
		{
			// Respawn upright
			FRotator Rot = GetActorRotation();
			Rot.Pitch = 0.f;
			Rot.Roll = 0.f;
			SetActorRotation(Rot);
			bSwitchStance = false;
			MovementComp->SkateVelocity = FVector::ZeroVector;
			MovementComp->bIsGrounded = true;
			SetState(ETH2SkaterState::Idle);
		}
		break;
	}

	// Wall bail check
	if (CurrentState != ETH2SkaterState::Bailing && MovementComp->GetCurrentSpeed() < 1.f && !MovementComp->IsGrounded())
	{
		// Velocity was zeroed by wall hit at high speed
		// (handled more precisely via hit events in a future iteration)
	}
}

void ATH2SkaterPawn::SetState(ETH2SkaterState NewState)
{
	if (CurrentState == NewState) return;

	CurrentState = NewState;

	if (NewState == ETH2SkaterState::Bailing)
	{
		BailTimer = BailDuration;
		MovementComp->SkateVelocity = FVector::ZeroVector;

		if (BailImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, BailImpactSound, GetActorLocation());
		}

		// Tilt capsule to simulate tumble
		FRotator Rot = GetActorRotation();
		Rot.Roll = 45.f;
		SetActorRotation(Rot);
	}
}

void ATH2SkaterPawn::UpdateCapsuleColor()
{
	if (!BodyMaterial)
	{
		if (BodyMesh && BodyMesh->GetMaterial(0))
		{
			BodyMaterial = BodyMesh->CreateAndSetMaterialInstanceDynamic(0);
		}
		if (!BodyMaterial) return;
	}

	FLinearColor TargetColor;
	switch (CurrentState)
	{
	case ETH2SkaterState::Idle:     TargetColor = IdleColor; break;
	case ETH2SkaterState::Pushing:  TargetColor = PushingColor; break;
	case ETH2SkaterState::Crouching: TargetColor = CrouchingColor; break;
	case ETH2SkaterState::Airborne: TargetColor = AirborneColor; break;
	case ETH2SkaterState::Landing:
	{
		float Alpha = LandingFlashTimer / LandingFlashDuration;
		TargetColor = FMath::Lerp(IdleColor, LandingColor, Alpha);
		break;
	}
	case ETH2SkaterState::Bailing:  TargetColor = BailColor; break;
	}

	// Switch stance tint
	if (bSwitchStance && CurrentState != ETH2SkaterState::Bailing)
	{
		TargetColor = FMath::Lerp(TargetColor, SwitchTintColor, 0.3f);
	}

	BodyMaterial->SetVectorParameterValue(TEXT("BaseColor"), TargetColor);
}

void ATH2SkaterPawn::UpdateCamera(float DeltaTime)
{
	if (!SpringArm) return;

	float TargetArmLength = DefaultArmLength;

	// Pull back when airborne off vert
	if (CurrentState == ETH2SkaterState::Airborne && MovementComp->GetVerticalVelocity() > 0.f)
	{
		TargetArmLength = VertArmLength;
	}

	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetArmLength, DeltaTime, 5.f);
}

void ATH2SkaterPawn::UpdateAudio()
{
	bool bShouldPlayWheelRoll = MovementComp->IsGrounded() && MovementComp->GetCurrentSpeed() > 30.f && CurrentState != ETH2SkaterState::Bailing;
	bool bShouldPlayWind = !MovementComp->IsGrounded() && CurrentState == ETH2SkaterState::Airborne;

	if (WheelRollAudio)
	{
		if (bShouldPlayWheelRoll && !WheelRollAudio->IsPlaying())
		{
			WheelRollAudio->Play();
		}
		else if (!bShouldPlayWheelRoll && WheelRollAudio->IsPlaying())
		{
			WheelRollAudio->Stop();
		}

		// Pitch based on speed
		if (bShouldPlayWheelRoll)
		{
			float SpeedRatio = MovementComp->GetCurrentSpeed() / MovementComp->MaxSpeed;
			WheelRollAudio->SetPitchMultiplier(FMath::Lerp(0.7f, 1.3f, SpeedRatio));
			WheelRollAudio->SetVolumeMultiplier(FMath::Lerp(0.3f, 1.0f, SpeedRatio));
		}
	}

	if (WindAirAudio)
	{
		if (bShouldPlayWind && !WindAirAudio->IsPlaying())
		{
			WindAirAudio->Play();
		}
		else if (!bShouldPlayWind && WindAirAudio->IsPlaying())
		{
			WindAirAudio->Stop();
		}
	}
}

void ATH2SkaterPawn::CheckAdvancedLaunch()
{
	double Now = GetWorld()->GetTimeSeconds();
	double WindowStart = Now - InputBufferWindow;

	// Count recent Up inputs
	int32 UpCount = 0;
	double LastUpTime = 0;
	for (const FInputBufferEntry& Entry : InputBuffer)
	{
		if (Entry.Timestamp >= WindowStart && Entry.Direction.Y > 0.5f)
		{
			UpCount++;
			LastUpTime = Entry.Timestamp;
		}
	}

	if (UpCount >= 2)
	{
		// Two Up taps within window → Boneless/Fastplant
		LastLaunchType = ETH2LaunchType::Boneless;
	}
	else if (UpCount == 1)
	{
		double TimeSinceUp = Now - LastUpTime;
		if (TimeSinceUp < 0.133) // ~8 frames at 60fps
		{
			// Up then X quickly → No Comply
			LastLaunchType = ETH2LaunchType::NoComply;
		}
		else
		{
			// Up + X with some delay → Nollie
			LastLaunchType = ETH2LaunchType::Nollie;
		}
	}
	else
	{
		LastLaunchType = ETH2LaunchType::Normal;
	}

	InputBuffer.Empty();
}
