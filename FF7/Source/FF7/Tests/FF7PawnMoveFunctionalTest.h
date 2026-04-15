// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "FF7PawnMoveFunctionalTest.generated.h"

class AFF7PlayerPawn;

/**
 * FF7.Pawn.MoveInputTranslates — place in a blank map.
 * Spawns an AFF7PlayerPawn, injects a +X move input for 0.5s, then asserts
 * the pawn translated in +X. Exercises the movement component path without
 * needing EnhancedInput asset wiring at runtime.
 */
UCLASS()
class AFF7PawnMoveFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	AFF7PawnMoveFunctionalTest();

protected:
	virtual void StartTest() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY()
	TObjectPtr<AFF7PlayerPawn> SpawnedPawn;

	FVector StartLocation = FVector::ZeroVector;
	float ElapsedSeconds = 0.0f;

	static constexpr float InputDurationSeconds = 0.5f;
	static constexpr float MinExpectedDeltaX = 1.0f;
};
