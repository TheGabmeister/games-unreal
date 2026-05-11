


#include "Camera/CBCamera.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Settings/CBWorldSettings.h"
#include "Player/CBPlayerCharacter.h"
#include "Player/CBCharacterMovementComponent.h"


ACBCamera::ACBCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	// We want to update the camera's position based on the follow target
	// after physics has been applied. This means the follow target will be in its
	// new position for this frame.
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	BlockingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockingMesh")); 
	CameraMovementTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CameraMovementTriggerBox")); 

	// Attach the components in the desired hierarchy
	RootComponent = DefaultSceneRoot; 
	SpringArmComponent->SetupAttachment(DefaultSceneRoot);
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraMovementTriggerBox->SetupAttachment(DefaultSceneRoot);
	CameraMovementTriggerBox->SetBoxExtent(FVector(100.0f, 500.0f, 500.0f));
	BlockingMesh->SetupAttachment(DefaultSceneRoot); 
}

void ACBCamera::BeginPlay()
{
	FollowCharacterMovementComponent = FollowTarget->GetCharacterMovement<UCBCharacterMovementComponent>(); 
	ensureMsgf(FollowCharacterMovementComponent, TEXT("ACBCamera::BeginPlay: CB Camera initialized with follow target but could not find UCBCharacterMovementComponent. Check your character setup.")); 
	ensureMsgf(CBWorldSettings, TEXT("ACBCamera::BeginPlay: CB Camera initialized invalid world settings. Check your world setup."));

	// Bind to the character's landed delagate
	FollowTarget->LandedDelegate.AddDynamic(this, &ACBCamera::OnCharacterLanded);

	// Bind to the trigger box's overlap delegates 
	CameraMovementTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ACBCamera::MovementTriggerBoxOverlapBegin);
	CameraMovementTriggerBox->OnComponentEndOverlap.AddDynamic(this, &ACBCamera::MovementTriggerBoxOverlapEnd);
	
	// The camera's Y should only change when the camera movement changes 
	CameraDefaultY = GetActorLocation().Y; 

	Super::BeginPlay(); 
}

void ACBCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind the trigger box's overlap delegates 
	CameraMovementTriggerBox->OnComponentBeginOverlap.RemoveDynamic(this, &ACBCamera::MovementTriggerBoxOverlapBegin);
	CameraMovementTriggerBox->OnComponentEndOverlap.RemoveDynamic(this, &ACBCamera::MovementTriggerBoxOverlapEnd);

	Super::EndPlay(EndPlayReason); 
}

void ACBCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); 

	switch (CameraMode)
	{
	case ECameraMode::Follow:
		TickFollowBehavior(DeltaTime); 
		break;
	case ECameraMode::Fixed:
		TickFixedPoint(DeltaTime); 
		break;
	case ECameraMode::None:
	default:
		break;
	}
}

void ACBCamera::TickFollowBehavior(float DeltaTime)
{
	if (!IsValid(FollowTarget) || !IsValid(FollowCharacterMovementComponent))
	{
		InterpolateToLastKnownFollowLocation(DeltaTime);
		return;
	}

	InterpolateFromFollowTarget(DeltaTime);
}

void ACBCamera::TickFixedPoint(float DeltaTime)
{
	if (bReachedFixedPoint)
	{
		return; 
	}

	const FVector& CurrentLocation = GetActorLocation(); 
	const FVector NewLocation = FMath::VInterpTo(CurrentLocation, FixedPointLocation, DeltaTime, FixedPointInterpolationSpeed);
	SetActorLocation(NewLocation); 

	if (FVector::DistSquared(CurrentLocation, NewLocation) <= UE_KINDA_SMALL_NUMBER)
	{
		bReachedFixedPoint = true; 
	}
}

void ACBCamera::CalculateLateralSpeed()
{
	// Get the relevant locations that we need to calculate the speed 
	const FVector& CurrentCameraLocation = GetActorLocation();
	const FVector& CameraMovementBoxLocation = CameraMovementTriggerBox->GetComponentLocation(); 
	const FVector& BoxExtents = CameraMovementTriggerBox->GetScaledBoxExtent(); 

	// Calculate the distance from the follow target to the left-most edge of the box 
	float BoxEdgeX = CameraMovementBoxLocation.X - BoxExtents.X; 
	float FollowTargetX = FollowTarget->GetActorLocation().X; 
	float TargetToEdgeDistance = FMath::Abs(FollowTargetX - BoxEdgeX); 

	// When the distance is less than zero, we don't want to follow along the X-axis 
	if (TargetToEdgeDistance < 0)
	{
		CurrentLateralSpeed = 0.0f; 
		return; 
	}

	// When the edge is crossed, we want to speed up the camera based on how far the player is into the box
	float DistanceMultiplier = (TargetToEdgeDistance / BoxExtents.X); 
	// We also apply a design authorable multiple
	// Then we multiply by the character's max walk speed. We always want the camera moving relative to the player's top speed. 
	CurrentLateralSpeed = DistanceMultiplier * LateralFollowSpeedMultiplier * FollowCharacterMovementComponent->MaxWalkSpeed;
}

void ACBCamera::InterpolateFromFollowTarget(float DeltaTime)
{
	const FVector& CurrentCameraLocation = GetActorLocation();
	const FVector& FollowTargetLocation = FollowTarget->GetActorLocation();

	const float FollowSpeed = LateralFollowSpeedMultiplier * FollowCharacterMovementComponent->MaxWalkSpeed;

	float StepX = FMath::FInterpTo(CurrentCameraLocation.X, FollowTargetLocation.X, DeltaTime, FollowZSpeed * 2.0f);
	float StepY = FMath::FInterpTo(CurrentCameraLocation.Y, FollowTargetLocation.Y, DeltaTime, FollowZSpeed * 2.0f);
	float StepZ = FMath::FInterpTo(CurrentCameraLocation.Z, FollowTargetLocation.Z, DeltaTime, FollowZSpeed);

	SetActorLocation(FVector(StepX, StepY, StepZ));
}

void ACBCamera::InterpolateToLastKnownFollowLocation(float DeltaTime)
{
	const FVector& CurrentCameraLocation = GetActorLocation();
	float StepZ = FMath::FInterpTo(CurrentCameraLocation.Z, FollowTargetZ, DeltaTime, FollowZSpeed);
	SetActorLocation(FVector(CurrentCameraLocation.X, CurrentCameraLocation.Y, StepZ));
}

void ACBCamera::MovementTriggerBoxOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (CameraMode != ECameraMode::Follow)
	{
		return; 
	}

	if (TObjectPtr<ACBPlayerCharacter> Player = Cast<ACBPlayerCharacter>(OtherActor))
	{
		if (Player != FollowTarget)
		{
			return;
		}

		bIsOverlappingMovementBox = true; 
	}
}

void ACBCamera::MovementTriggerBoxOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (TObjectPtr<ACBPlayerCharacter> Player = Cast<ACBPlayerCharacter>(OtherActor))
	{
		if (Player != FollowTarget)
		{
			return; 
		}

		bIsOverlappingMovementBox = false;

		if (CameraMode != ECameraMode::Follow)
		{
			return;
		}

		// Edge case
		// If the player somehow manages to exit beyond the center of the movement box we should snap the camera into place where they've exited, immediately
		if (FollowTarget->GetActorLocation().X > CameraMovementTriggerBox->GetComponentLocation().X)
		{
			const FVector& CurrentTargetLocation = FollowTarget->GetActorLocation();
			const FVector& CurrentCameraLocation = GetActorLocation(); 
			SetActorLocation(FVector(CurrentTargetLocation.X, CurrentCameraLocation.Y, CurrentTargetLocation.Z)); 
		}
	}
}

void ACBCamera::OnCharacterLanded(const FHitResult& Hit)
{
	const FVector& FollowTargetLocation = FollowTarget->GetActorLocation();
	FollowTargetZ = FollowTargetLocation.Z; 
}

void ACBCamera::SetCameraMode(ECameraMode NewMode)
{
	if (NewMode != CameraMode)
	{
		// Clean up state when exiting follow 
		if (CameraMode == ECameraMode::Follow && bIsOverlappingMovementBox)
		{
			bIsOverlappingMovementBox = false;
		}
		
		if (NewMode == ECameraMode::Fixed)
		{
			bReachedFixedPoint = false; 
		}
		else if (NewMode == ECameraMode::Follow)
		{
			// The player may have exited the fixed point camera beyond the follow movement box. 
			// In this case, we should interpolate as if they are exceeding it. 
			const FVector& BoxExtents = CameraMovementTriggerBox->GetScaledBoxExtent();
			const float MovementTriggerBoxEdgeX = CameraMovementTriggerBox->GetComponentLocation().X - BoxExtents.X;
			if (FollowTarget->GetActorLocation().X >= MovementTriggerBoxEdgeX)
			{
				bIsOverlappingMovementBox = true;
			}
		}
	}

	CameraMode = NewMode;
}

FVector ACBCamera::GetCameraComponentWorldPosition()
{
	return CameraComponent->GetComponentTransform().GetLocation();
}
