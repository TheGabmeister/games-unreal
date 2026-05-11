

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "Camera/CBCamera.h"
#include "CBWorldSettings.generated.h"

/**
 * Actor containing all script accessible world properties. 
 * For CB, these are object references that each level will need as we load in.  
 */
UCLASS()
class CB_API ACBWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
	
public:

	// The default music for audio subsystem to play for a world once all loading has completed
	// (Can be 'None')
	UPROPERTY(EditDefaultsOnly, Category="CB")
	TObjectPtr<USoundBase> WorldMusic; 

	// The default camera actor class that should spawn in the world as part of the camera subsystem 
	// (Can be 'None') 
	UPROPERTY(EditDefaultsOnly, Category="CB")
	TSubclassOf<AActor> CameraActorClass; 

	// The default camera behavior tick mode that the CB camera should use 
	// (Can be 'None') 
	UPROPERTY(EditDefaultsOnly, Category = "CB")
	ECameraMode DefaultCameraMode = ECameraMode::Follow;

	// The lower bound at which the camera will stop following player Z 
	UPROPERTY(EditDefaultsOnly, Category="CB")
	float CutoffLowerBoundZ = -400.0f; 

	// The upper bound at which the camera will stop following player Z 
	UPROPERTY(EditDefaultsOnly, Category="CB")
	float CutoffUpperBoundZ = 750.0f; 
};
