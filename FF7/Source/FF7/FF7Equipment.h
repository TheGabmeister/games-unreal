// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FF7Equipment.generated.h"

/**
 * Equipment stub (SPEC §2.9) — real fields land in Phase 7.
 * Exists now so FPartyMember can hold a pointer slot that stays null
 * until equipment and materia are wired up.
 */
UCLASS(BlueprintType)
class FF7_API UFF7Equipment : public UObject
{
	GENERATED_BODY()
};
