#pragma once
#include "NativeGameplayTags.h"
#include "CoreMinimal.h"
#include "CBEvents.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(EV_OnPickedUp_WumpaFruit);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(EV_OnPickedUp_ExtraLife);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(EV_OnPickedUp_AkuAku);

USTRUCT(BlueprintType)
struct FGameplayMessageBool
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Message Structs")
	bool Value = false;
};

USTRUCT(BlueprintType)
struct FGameplayMessageInt
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Message Structs")
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FGameplayMessageFloat
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Message Structs")
	float Value = 0.0f;
};

USTRUCT(BlueprintType)
struct FGameplayMessageString
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Message Structs")
	FString Value = TEXT("");
};