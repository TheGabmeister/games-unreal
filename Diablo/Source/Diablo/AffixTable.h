#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemInstance.h"
#include "AffixTable.generated.h"

USTRUCT(BlueprintType)
struct FAffixDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName AffixName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAffixType Type = EAffixType::BonusStr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MinValue = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxValue = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 QualityLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 GoldValueBonus = 50;
};

UCLASS(BlueprintType)
class DIABLO_API UAffixTable : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Affixes")
	TArray<FAffixDefinition> Entries;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("AffixTable", GetFName());
	}
};
