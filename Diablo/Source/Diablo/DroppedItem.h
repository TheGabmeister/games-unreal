#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DroppedItem.generated.h"

class UStaticMeshComponent;
class ADiabloHero;

UCLASS(Abstract)
class DIABLO_API ADroppedItem : public AActor
{
	GENERATED_BODY()

public:
	ADroppedItem();

	void OnPickedUp(ADiabloHero* Hero);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	float HealAmount = 50.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
