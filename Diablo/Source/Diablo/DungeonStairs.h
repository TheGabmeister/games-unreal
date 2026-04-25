#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonStairs.generated.h"

class UStaticMeshComponent;

UCLASS()
class DIABLO_API ADungeonStairs : public AActor
{
	GENERATED_BODY()

public:
	ADungeonStairs();

	void OnInteract();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stairs")
	FName TargetLevelName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stairs")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
