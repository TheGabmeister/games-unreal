// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FF7DialogueTypes.generated.h"

/**
 * One dialogue line (SPEC §2.4). DT_Dialogue is a DataTable keyed by the
 * row's RowName; `NextId` chains lines, `NAME_None` terminates.
 */
USTRUCT(BlueprintType)
struct FF7_API FDialogueLineRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Dialogue")
	FName SpeakerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Dialogue", meta = (MultiLine = "true"))
	FText Line;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Dialogue")
	FName NextId;
};
