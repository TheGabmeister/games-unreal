// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FF7PartyTypes.h"
#include "FF7CharacterDefinition.generated.h"

class UStaticMesh;

/**
 * Per-character data (SPEC §2.13). UPrimaryDataAsset so AssetManager can
 * enumerate and async-load by PrimaryAssetId (CharacterId).
 *
 * Phase 3 fills in the minimal fields needed to seed a roster; Phase 4
 * adds the level curve pointer, Phase 11 adds Limit branches, etc.
 */
UCLASS(BlueprintType)
class FF7_API UFF7CharacterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Unique ID shared with FPartyMember::CharacterId and FName row lookups. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FF7|Identity")
	FName CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FF7|Identity")
	FText DisplayName;

	/** Placeholder OBJ-generated static mesh (§3.2); swapped for skeletal later. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FF7|Visuals")
	TSoftObjectPtr<UStaticMesh> PlaceholderMesh;

	/** Starting stats applied when the roster is first populated. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FF7|Stats")
	FCharacterStats DefaultStats;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("FF7Character"), CharacterId);
	}
};
