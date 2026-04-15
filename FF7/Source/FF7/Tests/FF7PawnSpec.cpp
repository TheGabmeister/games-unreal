// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FF7PlayerPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FFF7PawnSpec, "FF7.Pawn.DefaultComponents",
	EAutomationTestFlags::EditorContext
	| EAutomationTestFlags::ClientContext
	| EAutomationTestFlags::ProductFilter)

	UWorld* TransientWorld = nullptr;
	AFF7PlayerPawn* PawnUnderTest = nullptr;

END_DEFINE_SPEC(FFF7PawnSpec)

void FFF7PawnSpec::Define()
{
	Describe("AFF7PlayerPawn spawned into a transient world", [this]()
	{
		BeforeEach([this]()
		{
			TransientWorld = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			TestNotNull(TEXT("Transient world created"), TransientWorld);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			PawnUnderTest = TransientWorld->SpawnActor<AFF7PlayerPawn>(AFF7PlayerPawn::StaticClass(), FTransform::Identity, Params);
			TestNotNull(TEXT("Pawn spawned"), PawnUnderTest);
		});

		It("has a capsule root", [this]()
		{
			if (!PawnUnderTest) return;
			TestNotNull(TEXT("Capsule"), Cast<UCapsuleComponent>(PawnUnderTest->GetRootComponent()));
		});

		It("has a spring arm component", [this]()
		{
			if (!PawnUnderTest) return;
			TestNotNull(TEXT("SpringArm"), PawnUnderTest->GetSpringArm());
		});

		It("has a camera component attached to the spring arm", [this]()
		{
			if (!PawnUnderTest) return;
			TestNotNull(TEXT("Camera"), PawnUnderTest->GetCamera());
			if (PawnUnderTest->GetCamera() && PawnUnderTest->GetSpringArm())
			{
				TestEqual(TEXT("Camera attached to SpringArm"),
					PawnUnderTest->GetCamera()->GetAttachParent(),
					static_cast<USceneComponent*>(PawnUnderTest->GetSpringArm()));
			}
		});

		AfterEach([this]()
		{
			if (TransientWorld)
			{
				TransientWorld->DestroyWorld(false);
				TransientWorld = nullptr;
			}
			PawnUnderTest = nullptr;
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS
