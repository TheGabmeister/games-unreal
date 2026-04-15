// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FF7CharacterBase.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class UStaticMesh;

/**
 * Shared base for every in-world character (player, party members, enemies).
 * SPEC §2.2: capsule root + visual pivot + placeholder static mesh.
 * Resolving the placeholder mesh on BeginPlay keeps the soft-ref pattern the
 * whole project uses; a null ref falls back to the engine cube so the pawn
 * is visible during development.
 */
UCLASS(Abstract)
class FF7_API AFF7CharacterBase : public APawn
{
	GENERATED_BODY()

public:
	AFF7CharacterBase();

	UCapsuleComponent* GetCapsule() const { return Capsule; }
	USceneComponent* GetVisualPivot() const { return VisualPivot; }
	UStaticMeshComponent* GetMesh() const { return Mesh; }

protected:
	virtual void BeginPlay() override;

	/** Soft ref to the placeholder static mesh (§3.2). Assign on BP subclass or character definition. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FF7|Visuals")
	TSoftObjectPtr<UStaticMesh> PlaceholderMesh;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FF7|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FF7|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> VisualPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FF7|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

	void ResolveAndApplyPlaceholderMesh();
};
