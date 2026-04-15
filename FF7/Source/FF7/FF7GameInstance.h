// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FF7PartyTypes.h"
#include "FF7GameInstance.generated.h"

class UFF7CharacterDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGilChanged, int32, NewGil);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfigChanged);

/**
 * Project-wide persistent state (SPEC §2.5). Survives level loads; owns the
 * roster, gil, world flags, and player config. Domains grow into their own
 * UObjects (UFF7Inventory, UFF7MateriaPool, etc.) per the SPEC extraction
 * rule — Phase 3 keeps everything here except future Inventory (Phase 5).
 */
UCLASS(BlueprintType, Config = Game)
class FF7_API UFF7GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	/** Defaults assigned on BP_FF7GameInstance. 9 slots drive Roster. */
	UPROPERTY(EditDefaultsOnly, Category = "FF7|Party")
	TArray<TSoftObjectPtr<UFF7CharacterDefinition>> DefaultRoster;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FF7|Party")
	TArray<FPartyMember> Roster;

	/** Indices into Roster — exactly 3, no duplicates. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "FF7|Party")
	TArray<int32> ActivePartyIndices;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FF7|Economy")
	int32 Gil = 0;

	UPROPERTY(BlueprintAssignable, Category = "FF7|Economy")
	FOnGilChanged OnGilChanged;

	UPROPERTY(Transient)
	TMap<FName, int32> WorldFlags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FF7|Config")
	FGameConfig Config;

	UPROPERTY(BlueprintAssignable, Category = "FF7|Config")
	FOnConfigChanged OnConfigChanged;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "FF7|Session")
	float PlayTimeSeconds = 0.0f;

	/** Populate Roster from `Defs`; pads to 9 entries with empty FPartyMember. */
	void InitializeRoster(const TArray<UFF7CharacterDefinition*>& Defs);

	/** Gil economy. Add ignores non-positive amounts; Spend returns false on insufficient funds. */
	UFUNCTION(BlueprintCallable, Category = "FF7|Economy")
	void AddGil(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "FF7|Economy")
	bool SpendGil(int32 Amount);

	/** Place Roster[RosterIdx] at ActivePartyIndices[SlotIdx]. Rejects duplicate actives. */
	UFUNCTION(BlueprintCallable, Category = "FF7|Party")
	bool SwapActive(int32 SlotIdx, int32 RosterIdx);

	/** Mutate and broadcast config. Use instead of assigning Config directly. */
	UFUNCTION(BlueprintCallable, Category = "FF7|Config")
	void SetConfig(const FGameConfig& NewConfig);

	// ------- Exec (console cheat commands) -------
	// UE exec function names don't allow dots, so the console form uses underscores.

	UFUNCTION(Exec)
	void FF7PartyDump();

	UFUNCTION(Exec)
	void FF7PartySwapActive(int32 SlotIdx, int32 RosterIdx);

	UFUNCTION(Exec)
	void FF7GilAdd(int32 Amount);
};
