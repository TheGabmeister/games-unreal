#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "DiabloNPC.generated.h"

class UStaticMeshComponent;
class UNPCShopData;

UENUM(BlueprintType)
enum class ENPCType : uint8
{
	None,
	Merchant,
	Healer,
	Identifier
};

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	ENPCType NPCType = ENPCType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<UNPCShopData> ShopData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
