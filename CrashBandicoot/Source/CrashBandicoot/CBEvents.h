#pragma once
#include "NativeGameplayTags.h"
#include "CoreMinimal.h"
#include "CBEvents.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(Events_OnPickedUp_WumpaFruit);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Events_OnPickedUp_ExtraLife);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Events_OnPickedUp_AkuAku);

// Example usage:
// FGameplayTag Channel = FGameplayTag::RequestGameplayTag(TEXT("Pickup.Collected"));
// OR
// UPROPERTY(EditAnywhere, Category = "Default", meta = (AllowPrivateAccess = "true"))
// FGameplayTag Channel;

// Example usage of broadcasting a message
// FGameplayMessageInt Message {10};
// UGameplayMessageSubsystem::Get(this).BroadcastMessage(Channel, Message);
// OR
// UGameplayMessageSubsystem::Get(this).BroadcastMessage(Events_OnPickedUp_WumpaFruit, Message);

USTRUCT(BlueprintType)
struct FGameplayMessageBool
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Message Structs")
	bool Value;
};

USTRUCT(BlueprintType)
struct FGameplayMessageInt
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Message Structs")
	int32 Value;
};

USTRUCT(BlueprintType)
struct FGameplayMessageFloat
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Message Structs")
	float Value;
};

USTRUCT(BlueprintType)
struct FGameplayMessageString
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gameplay Message Structs")
	FString Value;
};