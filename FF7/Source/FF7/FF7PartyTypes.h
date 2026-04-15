// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FF7Equipment.h"
#include "FF7PartyTypes.generated.h"

/** Per-character base stats (SPEC §2.6). Curves land in Phase 4; this is the storage shape. */
USTRUCT(BlueprintType)
struct FF7_API FCharacterStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Stats") int32 HP = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Stats") int32 MaxHP = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Stats") int32 MP = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Stats") int32 MaxMP = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Stats") int32 Str = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Stats") int32 Vit = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Stats") int32 Mag = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Stats") int32 Spr = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Stats") int32 Dex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Stats") int32 Lck = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Stats") int32 Level = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Stats") int32 EXP = 0;
};

/** Limit Break gauge (SPEC §2.13) — stub today; Phase 11 fills in the mechanics. */
USTRUCT(BlueprintType)
struct FF7_API FLimitGauge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Limit") uint8 Fill = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Limit") uint8 TierUnlocked = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Limit") uint8 ActiveBranch = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Limit") int32 KillCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Limit") int32 BranchUseCount = 0;
};

/** Party member shell (SPEC §2.5). Equipment stays null until Phase 7. */
USTRUCT(BlueprintType)
struct FF7_API FPartyMember
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Party")
	FName CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Party")
	FCharacterStats Stats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Party")
	TObjectPtr<UFF7Equipment> Equipment = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Party")
	FLimitGauge Limit;
};

UENUM(BlueprintType)
enum class EATBMode : uint8
{
	Active,
	Wait,
};

/** Player settings (SPEC §2.5). Mutation fires UFF7GameInstance::OnConfigChanged. */
USTRUCT(BlueprintType)
struct FF7_API FGameConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Config") EATBMode ATBMode = EATBMode::Active;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Config") int32 ATBSpeed = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Config") float BGMVolume = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Config") float SFXVolume = 1.0f;
};
