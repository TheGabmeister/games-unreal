// Copyright Epic Games, Inc. All Rights Reserved.

#include "CrashBandicootGameMode.h"
#include "CrashBandicootCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACrashBandicootGameMode::ACrashBandicootGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
