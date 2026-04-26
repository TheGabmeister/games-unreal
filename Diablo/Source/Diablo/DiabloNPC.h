#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "DiabloNPC.generated.h"

class UStaticMeshComponent;

UCLASS()
class DIABLO_API ADiabloNPC : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	ADiabloNPC();

	virtual void Interact(AActor* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FText NPCName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FText DialogText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
