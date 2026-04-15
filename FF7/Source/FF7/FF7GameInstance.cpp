// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF7GameInstance.h"
#include "FF7CharacterDefinition.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogFF7Party, Log, All);

static constexpr int32 RosterSize = 9;
static constexpr int32 ActivePartySize = 3;

void UFF7GameInstance::Init()
{
	Super::Init();

	TArray<UFF7CharacterDefinition*> Resolved;
	Resolved.Reserve(DefaultRoster.Num());
	for (const TSoftObjectPtr<UFF7CharacterDefinition>& SoftDef : DefaultRoster)
	{
		Resolved.Add(SoftDef.LoadSynchronous());
	}
	InitializeRoster(Resolved);

	ActivePartyIndices = { 0, 1, 2 };
	Gil = 0;
}

void UFF7GameInstance::InitializeRoster(const TArray<UFF7CharacterDefinition*>& Defs)
{
	Roster.Reset();
	Roster.Reserve(RosterSize);

	for (UFF7CharacterDefinition* Def : Defs)
	{
		FPartyMember Member;
		if (Def)
		{
			Member.CharacterId = Def->CharacterId;
			Member.Stats = Def->DefaultStats;
		}
		Roster.Add(Member);
		if (Roster.Num() >= RosterSize)
		{
			break;
		}
	}
	while (Roster.Num() < RosterSize)
	{
		Roster.Add(FPartyMember{});
	}

	if (ActivePartyIndices.Num() != ActivePartySize)
	{
		ActivePartyIndices = { 0, 1, 2 };
	}
}

void UFF7GameInstance::AddGil(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}
	Gil += Amount;
	OnGilChanged.Broadcast(Gil);
}

bool UFF7GameInstance::SpendGil(int32 Amount)
{
	if (Amount <= 0 || Gil < Amount)
	{
		return false;
	}
	Gil -= Amount;
	OnGilChanged.Broadcast(Gil);
	return true;
}

bool UFF7GameInstance::SwapActive(int32 SlotIdx, int32 RosterIdx)
{
	if (!ActivePartyIndices.IsValidIndex(SlotIdx) || !Roster.IsValidIndex(RosterIdx))
	{
		return false;
	}

	for (int32 i = 0; i < ActivePartyIndices.Num(); ++i)
	{
		if (i != SlotIdx && ActivePartyIndices[i] == RosterIdx)
		{
			return false;
		}
	}

	ActivePartyIndices[SlotIdx] = RosterIdx;
	return true;
}

void UFF7GameInstance::SetConfig(const FGameConfig& NewConfig)
{
	Config = NewConfig;
	OnConfigChanged.Broadcast();
}

// ---------- Exec ---------- //

void UFF7GameInstance::FF7PartyDump()
{
	UE_LOG(LogFF7Party, Display, TEXT("---- FF7 Party Dump ----"));
	UE_LOG(LogFF7Party, Display, TEXT("Gil: %d"), Gil);
	UE_LOG(LogFF7Party, Display, TEXT("Active: [%s]"), *FString::JoinBy(ActivePartyIndices, TEXT(","),
		[](int32 Idx) { return FString::FromInt(Idx); }));
	for (int32 i = 0; i < Roster.Num(); ++i)
	{
		const FPartyMember& M = Roster[i];
		UE_LOG(LogFF7Party, Display, TEXT("  [%d] %s  Lv%d  HP %d/%d  MP %d/%d"),
			i, *M.CharacterId.ToString(),
			M.Stats.Level, M.Stats.HP, M.Stats.MaxHP, M.Stats.MP, M.Stats.MaxMP);
	}
}

void UFF7GameInstance::FF7PartySwapActive(int32 SlotIdx, int32 RosterIdx)
{
	const bool bOK = SwapActive(SlotIdx, RosterIdx);
	UE_LOG(LogFF7Party, Display, TEXT("SwapActive slot=%d roster=%d -> %s"),
		SlotIdx, RosterIdx, bOK ? TEXT("OK") : TEXT("REJECTED"));
}

void UFF7GameInstance::FF7GilAdd(int32 Amount)
{
	AddGil(Amount);
	UE_LOG(LogFF7Party, Display, TEXT("Gil = %d"), Gil);
}
