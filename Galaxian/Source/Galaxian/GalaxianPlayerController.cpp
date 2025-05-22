// Fill out your copyright notice in the Description page of Project Settings.


#include "GalaxianPlayerController.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"

AGalaxianPlayerController::AGalaxianPlayerController()
{
	bAutoManageActiveCameraTarget = false;

    
}

void AGalaxianPlayerController::BeginPlay()
{
    // Find the camera actor in the level (by tag or name for reliability)
    for (TActorIterator<ACameraActor> It(GetWorld()); It; ++It)
    {
        ACameraActor* Camera = *It;
        if (Camera && Camera->ActorHasTag(TEXT("MainCamera")))
        {
            SetViewTarget(Camera);
            break;
        }
    }
}