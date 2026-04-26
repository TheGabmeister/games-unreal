#pragma once

#include "CoreMinimal.h"
#include "ItemDefinition.h"
#include "ItemInstance.generated.h"

UENUM(BlueprintType)
enum class EAffixType : uint8
{
	BonusStr,
	BonusMag,
	BonusDex,
	BonusVit,
	BonusDamage,
	BonusArmor,
	BonusHP,
	BonusMana,
	BonusToHit
};

USTRUCT(BlueprintType)
struct FItemAffix
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AffixName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAffixType Type = EAffixType::BonusStr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPrefix = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldValueBonus = 0;
};

USTRUCT(BlueprintType)
struct FItemInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UItemDefinition> Definition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<FItemAffix> Affixes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bIdentified = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 CurrentDurability = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 StackCount = 1;

	bool IsValid() const { return Definition != nullptr; }
};
