

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CBCameraSubsystem.generated.h"

class ACBWorldSettings; 
class ACBCamera; 
class ACBPlayerCharacter; 
enum class ECameraMode : uint8; 

// Log category for the CB Camera Subsystem 
DECLARE_LOG_CATEGORY_EXTERN(LogCBCameraSubsystem, Log, All); 

/**
* 
* A subsystem that interacts with the game camera.
* Shares the lifetime of the current world. 
* 
*/
UCLASS()
class CB_API UCBCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public: 

	// Invoked when Begin Play is called on the world
	void OnWorldBeginPlay(UWorld& InWorld) override;

	// Gets the world position of the camera component 
	UFUNCTION(BlueprintCallable, Category = "CB")
	FVector GetCameraWorldPosition();

	// Sets the camera's movement mode 
	UFUNCTION(BlueprintCallable, Category = "CB")
	void SetCameraMode(ECameraMode NewMode); 

	// Sets the fixed location that the camera should interpolate to 
	UFUNCTION(BlueprintCallable, Category = "CB")
	void SetCameraFixedPointTarget(const FVector& TargetPosition);

	// Returns true when the player character is the owning player of this camera system 
	UFUNCTION(BlueprintCallable, Category = "CB")
	bool IsPlayerCameraOwner(ACBPlayerCharacter* TargetPlayer); 

protected:

	// Spawns the camera from the camera class and initializes blend with local player
	void SetupCamera(); 

	// The settings used by the current world
	TObjectPtr<ACBWorldSettings> CBWorldSettings; 

	// The camera actor class to spawn based on the world settings 
	TSubclassOf<ACBCamera> CameraActorClass;

	// The instance of the camera actor 
	TObjectPtr<ACBCamera> CameraActorInstance;

	// The pawn that is the focus of this camera 
	TObjectPtr<ACBPlayerCharacter> CameraOwner; 
};
