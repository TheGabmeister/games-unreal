#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NPCShopData.generated.h"

class UItemDefinition;

UCLASS(BlueprintType)
class DIABLO_API UNPCShopData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shop")
	TArray<TObjectPtr<UItemDefinition>> StockItems;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("NPCShopData", GetFName());
	}
};
