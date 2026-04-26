#include "AffixGenerator.h"
#include "AffixTable.h"
#include "ItemInstance.h"
#include "ItemDefinition.h"
#include "Diablo.h"

static FItemAffix RollAffix(const UAffixTable* Table, int32 MonsterLevel, bool bIsPrefix)
{
	FItemAffix Result;

	if (!Table || Table->Entries.Num() == 0)
	{
		return Result;
	}

	TArray<const FAffixDefinition*> Eligible;
	for (const FAffixDefinition& Def : Table->Entries)
	{
		if (Def.QualityLevel <= MonsterLevel)
		{
			Eligible.Add(&Def);
		}
	}

	if (Eligible.Num() == 0)
	{
		return Result;
	}

	const FAffixDefinition& Picked = *Eligible[FMath::RandRange(0, Eligible.Num() - 1)];

	Result.AffixName = Picked.AffixName;
	Result.Type = Picked.Type;
	Result.Value = FMath::RoundToFloat(FMath::FRandRange(Picked.MinValue, Picked.MaxValue));
	Result.bIsPrefix = bIsPrefix;
	Result.GoldValueBonus = Picked.GoldValueBonus;

	return Result;
}

void FAffixGenerator::TryMakeMagic(FItemInstance& Item, int32 MonsterLevel, float MagicChance)
{
	if (!Item.Definition)
	{
		return;
	}

	const EItemCategory Cat = Item.Definition->Category;
	if (Cat == EItemCategory::Potion || Cat == EItemCategory::Scroll ||
		Cat == EItemCategory::Gold || Cat == EItemCategory::Misc)
	{
		return;
	}

	if (FMath::FRand() > MagicChance)
	{
		return;
	}

	const UAffixTable* PrefixTable = GetPrefixTable();
	const UAffixTable* SuffixTable = GetSuffixTable();

	bool bHasPrefix = FMath::FRand() < 0.5f;
	bool bHasSuffix = FMath::FRand() < 0.5f;

	if (!bHasPrefix && !bHasSuffix)
	{
		if (FMath::FRand() < 0.5f)
			bHasPrefix = true;
		else
			bHasSuffix = true;
	}

	if (bHasPrefix)
	{
		FItemAffix Prefix = RollAffix(PrefixTable, MonsterLevel, true);
		if (!Prefix.AffixName.IsNone())
		{
			Item.Affixes.Add(Prefix);
		}
	}

	if (bHasSuffix)
	{
		FItemAffix Suffix = RollAffix(SuffixTable, MonsterLevel, false);
		if (!Suffix.AffixName.IsNone())
		{
			Item.Affixes.Add(Suffix);
		}
	}

	if (Item.Affixes.Num() > 0)
	{
		Item.bIdentified = false;
	}
}

FString FAffixGenerator::GetDisplayName(const FItemInstance& Item)
{
	if (!Item.Definition)
	{
		return TEXT("Unknown");
	}

	const FString BaseName = Item.Definition->DisplayName.ToString();

	if (Item.Affixes.Num() == 0)
	{
		return BaseName;
	}

	if (!Item.bIdentified)
	{
		return BaseName;
	}

	FString Prefix;
	FString Suffix;

	for (const FItemAffix& Affix : Item.Affixes)
	{
		if (Affix.bIsPrefix)
		{
			Prefix = Affix.AffixName.ToString();
		}
		else
		{
			Suffix = Affix.AffixName.ToString();
		}
	}

	FString Result;
	if (!Prefix.IsEmpty())
	{
		Result = Prefix + TEXT(" ") + BaseName;
	}
	else
	{
		Result = BaseName;
	}

	if (!Suffix.IsEmpty())
	{
		Result += TEXT(" ") + Suffix;
	}

	return Result;
}

int32 FAffixGenerator::GetTotalGoldValue(const FItemInstance& Item)
{
	if (!Item.Definition)
	{
		return 0;
	}

	int32 Total = Item.Definition->GoldValue;
	for (const FItemAffix& Affix : Item.Affixes)
	{
		Total += Affix.GoldValueBonus;
	}
	return Total;
}

const UAffixTable* FAffixGenerator::GetPrefixTable()
{
	static TWeakObjectPtr<const UAffixTable> Cached;
	if (!Cached.IsValid())
	{
		Cached = LoadObject<UAffixTable>(nullptr,
			TEXT("/Game/Items/Affixes/AT_Prefixes.AT_Prefixes"));
	}
	return Cached.Get();
}

const UAffixTable* FAffixGenerator::GetSuffixTable()
{
	static TWeakObjectPtr<const UAffixTable> Cached;
	if (!Cached.IsValid())
	{
		Cached = LoadObject<UAffixTable>(nullptr,
			TEXT("/Game/Items/Affixes/AT_Suffixes.AT_Suffixes"));
	}
	return Cached.Get();
}
