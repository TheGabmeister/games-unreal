#pragma once

#include "CoreMinimal.h"

struct FItemInstance;
class UAffixTable;

struct FAffixGenerator
{
	static void TryMakeMagic(FItemInstance& Item, int32 MonsterLevel, float MagicChance = 0.25f);

	static FString GetDisplayName(const FItemInstance& Item);

	static int32 GetTotalGoldValue(const FItemInstance& Item);

	static const UAffixTable* GetPrefixTable();
	static const UAffixTable* GetSuffixTable();
};
