// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF7PawnMoveFunctionalTest.h"
#include "Engine/World.h"
#include "FF7PlayerPawn.h"

AFF7PawnMoveFunctionalTest::AFF7PawnMoveFunctionalTest()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	TimeLimit = 5.0f;
}

void AFF7PawnMoveFunctionalTest::StartTest()
{
	Super::StartTest();

	UWorld* World = GetWorld();
	if (!World)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("No world."));
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnedPawn = World->SpawnActor<AFF7PlayerPawn>(AFF7PlayerPawn::StaticClass(), GetActorTransform(), Params);
	if (!SpawnedPawn)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Failed to spawn AFF7PlayerPawn."));
		return;
	}

	StartLocation = SpawnedPawn->GetActorLocation();
	ElapsedSeconds = 0.0f;
}

void AFF7PawnMoveFunctionalTest::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsRunning() || !SpawnedPawn)
	{
		return;
	}

	ElapsedSeconds += DeltaSeconds;

	if (ElapsedSeconds < InputDurationSeconds)
	{
		SpawnedPawn->AddMovementInput(FVector(1.0f, 0.0f, 0.0f), 1.0f);
		return;
	}

	const float DeltaX = SpawnedPawn->GetActorLocation().X - StartLocation.X;
	if (DeltaX > MinExpectedDeltaX)
	{
		FinishTest(EFunctionalTestResult::Succeeded,
			FString::Printf(TEXT("Pawn translated %.1f along +X."), DeltaX));
	}
	else
	{
		FinishTest(EFunctionalTestResult::Failed,
			FString::Printf(TEXT("Pawn X-delta %.3f ≤ expected %.1f."), DeltaX, MinExpectedDeltaX));
	}
}
