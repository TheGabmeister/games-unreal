#include "QuakeSaveArchive.h"

#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

namespace QuakeSaveArchive
{
	void WriteSaveProperties(UObject* Object, TArray<uint8>& OutBytes)
	{
		if (!Object)
		{
			return;
		}
		FMemoryWriter MemWriter(OutBytes, /*bIsPersistent*/ true);
		FObjectAndNameAsStringProxyArchive Ar(MemWriter, /*bInLoadIfFindFails*/ true);
		// ArIsSaveGame filters to CPF_SaveGame — only UPROPERTYs marked with
		// meta = (SaveGame) participate. Without this the archive would also
		// write transient fields and bloat payloads.
		Ar.ArIsSaveGame = true;
		Object->Serialize(Ar);
	}

	void ReadSaveProperties(UObject* Object, const TArray<uint8>& InBytes)
	{
		if (!Object || InBytes.Num() == 0)
		{
			return;
		}
		FMemoryReader MemReader(InBytes, /*bIsPersistent*/ true);
		FObjectAndNameAsStringProxyArchive Ar(MemReader, /*bInLoadIfFindFails*/ true);
		Ar.ArIsSaveGame = true;
		Object->Serialize(Ar);
	}

	TArray<FName> ComputeConsumedNames(
		const TArray<FName>& InitialNames,
		const TArray<FName>& LiveNames)
	{
		TArray<FName> Result;
		Result.Reserve(InitialNames.Num());
		for (const FName& Name : InitialNames)
		{
			if (!LiveNames.Contains(Name))
			{
				Result.Add(Name);
			}
		}
		return Result;
	}

	bool CanQuickSave(EMovementMode MovementMode, bool bIsInPain, bool bIsDead)
	{
		return MovementMode == MOVE_Walking && !bIsInPain && !bIsDead;
	}

	double ComputeRestoredLevelStartTime(double WorldTimeNow, float ElapsedAtSave)
	{
		return WorldTimeNow - static_cast<double>(ElapsedAtSave);
	}
}
