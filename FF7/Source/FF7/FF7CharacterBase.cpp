// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF7CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

AFF7CharacterBase::AFF7CharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(40.0f, 88.0f);
	Capsule->SetCollisionProfileName(TEXT("Pawn"));
	RootComponent = Capsule;

	VisualPivot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualPivot"));
	VisualPivot->SetupAttachment(Capsule);
	VisualPivot->SetRelativeLocation(FVector(0.0f, 0.0f, -Capsule->GetScaledCapsuleHalfHeight()));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	Mesh->SetupAttachment(VisualPivot);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFF7CharacterBase::BeginPlay()
{
	Super::BeginPlay();
	ResolveAndApplyPlaceholderMesh();
}

void AFF7CharacterBase::ResolveAndApplyPlaceholderMesh()
{
	if (!Mesh)
	{
		return;
	}

	UStaticMesh* Resolved = PlaceholderMesh.IsNull() ? nullptr : PlaceholderMesh.LoadSynchronous();
	if (!Resolved)
	{
		// Fallback: engine cube so the pawn is visible even without assigned art.
		Resolved = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}

	Mesh->SetStaticMesh(Resolved);
}
