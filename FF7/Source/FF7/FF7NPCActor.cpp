// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF7NPCActor.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "FF7CollisionChannels.h"
#include "FF7PlayerController.h"

AFF7NPCActor::AFF7NPCActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(40.0f, 88.0f);
	Capsule->SetCollisionProfileName(TEXT("Pawn"));
	RootComponent = Capsule;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	Mesh->SetupAttachment(Capsule);
	Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractVolume = CreateDefaultSubobject<USphereComponent>(TEXT("InteractVolume"));
	InteractVolume->SetupAttachment(Capsule);
	InteractVolume->InitSphereRadius(150.0f);
	InteractVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	// Respond to the custom Interact trace channel (see FF7CollisionChannels.h).
	InteractVolume->SetCollisionResponseToChannel(FF7::ECC_Interact, ECR_Block);
}

void AFF7NPCActor::Interact_Implementation(AFF7PlayerController* Interactor)
{
	if (!Interactor || !DialogueTable || StartRowId.IsNone())
	{
		return;
	}

	Interactor->StartDialogue(DialogueTable, StartRowId);
}
