#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemInstance.h"
#include "DroppedItem.generated.h"

class UStaticMeshComponent;
class ADiabloHero;

UCLASS()
class DIABLO_API ADroppedItem : public AActor
{
	GENERATED_BODY()

public:
	ADroppedItem();

	void OnPickedUp(ADiabloHero* Hero);
	void InitFromItem(const FItemInstance& InItem);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	float HealAmount = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FItemInstance ItemData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
