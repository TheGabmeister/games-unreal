#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "DungeonStairs.generated.h"

class UStaticMeshComponent;

UCLASS()
class DIABLO_API ADungeonStairs : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	ADungeonStairs();

	virtual void Interact(AActor* Interactor) override;
	void OnInteract();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stairs")
	FName TargetLevelName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stairs")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
