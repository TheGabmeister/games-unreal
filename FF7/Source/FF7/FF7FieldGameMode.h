// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FF7FieldGameMode.generated.h"

/**
 * Field-level GameMode (SPEC §2.3). Defaults its pawn + controller classes
 * to the project's field types so an empty map just works with no per-map
 * overrides. A BP subclass (BP_FF7FieldGameMode) swaps in BP_ variants.
 */
UCLASS()
class FF7_API AFF7FieldGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFF7FieldGameMode();
};
