


#include "Camera/CBCameraSubsystem.h"
#include "Camera/CBCamera.h"
#include "Settings/CBWorldSettings.h"
#include "Player/CBPlayerCharacter.h"

DEFINE_LOG_CATEGORY(LogCBCameraSubsystem)

void UCBCameraSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	CBWorldSettings = Cast<ACBWorldSettings>(InWorld.GetWorldSettings()); 

	if (!CBWorldSettings)
	{
		UE_LOG(LogCBCameraSubsystem, Error, TEXT("UCBCameraSubsystem::OnWorldBeginPlay, could not get world settings - check your map settings %s"), *InWorld.GetName()); 
		return; 
	}

	CameraActorClass = CBWorldSettings->CameraActorClass; 

	if (!CameraActorClass)
	{
		// This could be intentional and implies that this map should manage its own camera. i.e. The Main Menu map
		UE_LOG(LogCBCameraSubsystem, Log, TEXT("UCBCameraSubsystem::OnWorldBeginPlay, no camera class found. Ignoring camera management in %s"), *InWorld.GetName()); 
		return; 
	}

	SetupCamera(); 
}

FVector UCBCameraSubsystem::GetCameraWorldPosition()
{
	return IsValid(CameraActorInstance) ? CameraActorInstance->GetCameraComponentWorldPosition() : FVector::ZeroVector;
}

void UCBCameraSubsystem::SetCameraMode(ECameraMode NewMode)
{
	CameraActorInstance->SetCameraMode(NewMode); 
}

void UCBCameraSubsystem::SetCameraFixedPointTarget(const FVector& TargetPosition)
{
	CameraActorInstance->SetCameraFixedPointTarget(TargetPosition); 
}

bool UCBCameraSubsystem::IsPlayerCameraOwner(ACBPlayerCharacter* TargetPlayer)
{
	return TargetPlayer == CameraOwner;
}

void UCBCameraSubsystem::SetupCamera()
{
	// Get our pawn and pawn location from the local player controller 
	TObjectPtr<UWorld> World = GetWorld();
	TObjectPtr<APlayerController> PlayerController = World->GetFirstLocalPlayerFromController()->GetPlayerController(World);
	TObjectPtr<APawn> Pawn = PlayerController->GetPawn();
	FVector PawnLocation = Pawn->GetActorLocation();

	// Defer spawn our camera, assign the follow target, then finish spawning 
	FTransform SpawnTransform = FTransform(PawnLocation);
	CameraActorInstance = World->SpawnActorDeferred<ACBCamera>(CameraActorClass.Get(), SpawnTransform);

	CameraActorInstance->SetWorldSettings(CBWorldSettings);
	CameraActorInstance->SetCameraMode(CBWorldSettings->DefaultCameraMode); 

	// Always set the follow target based on the camera's owner 
	if (TObjectPtr<ACBPlayerCharacter> TargetPlayer = Cast<ACBPlayerCharacter>(Pawn))
	{
		CameraOwner = TargetPlayer; 
		CameraActorInstance->SetFollowTarget(TargetPlayer);
	}
	else
	{
		UE_LOG(LogCBCameraSubsystem, Error, TEXT("UCBCameraSubsystem::SetupCamera, Could not cast pawn to CB Player Character. Check character setup."));
	}
	
	CameraActorInstance->FinishSpawning(SpawnTransform);

	// Blend the player controller's view target with the newly spawned camera 
	PlayerController->SetViewTargetWithBlend(CameraActorInstance);
}
