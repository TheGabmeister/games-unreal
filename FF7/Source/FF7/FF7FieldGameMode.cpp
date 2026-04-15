// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF7FieldGameMode.h"
#include "FF7PlayerController.h"
#include "FF7PlayerPawn.h"

AFF7FieldGameMode::AFF7FieldGameMode()
{
	DefaultPawnClass = AFF7PlayerPawn::StaticClass();
	PlayerControllerClass = AFF7PlayerController::StaticClass();
}
