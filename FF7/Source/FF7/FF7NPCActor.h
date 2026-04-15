// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FF7Interactable.h"
#include "GameFramework/Actor.h"
#include "FF7NPCActor.generated.h"

class UCapsuleComponent;
class UDataTable;
class USphereComponent;
class UStaticMeshComponent;

/**
 * NPC actor (SPEC §2.4). Capsule body collision, placeholder static mesh,
 * and a USphereComponent interact volume. Carries the DataTable + start row
 * it drives dialogue from; interaction delegates to the controller.
 */
UCLASS()
class FF7_API AFF7NPCActor : public AActor, public IFF7Interactable
{
	GENERATED_BODY()

public:
	AFF7NPCActor();

	/** DataTable of FDialogueLineRow driving this NPC. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Dialogue")
	TObjectPtr<UDataTable> DialogueTable;

	/** Row name the dialogue starts at when the player interacts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Dialogue")
	FName StartRowId;

	virtual void Interact_Implementation(AFF7PlayerController* Interactor) override;

	UCapsuleComponent* GetCapsule() const { return Capsule; }
	UStaticMeshComponent* GetMesh() const { return Mesh; }
	USphereComponent* GetInteractVolume() const { return InteractVolume; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FF7|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FF7|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FF7|Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> InteractVolume;
};
