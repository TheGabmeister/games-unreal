#pragma once

#include "CoreMinimal.h"
#include "CBStructs.generated.h"

USTRUCT(BlueprintType)
struct FGameplayMessageInt
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Message Structs")
	int32 Value;
};