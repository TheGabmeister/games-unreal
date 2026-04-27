#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "PortalActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class DIABLO_API APortalActor : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	APortalActor();

	virtual void Interact(AActor* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	bool bReturnsToDungeon = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
