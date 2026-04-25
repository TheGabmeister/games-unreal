#pragma once

#include "CoreMinimal.h"
#include "ItemDefinition.h"
#include "ItemInstance.generated.h"

USTRUCT(BlueprintType)
struct FItemAffix
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AffixName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;
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
	int32 CurrentDurability = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 StackCount = 1;

	bool IsValid() const { return Definition != nullptr; }
};
