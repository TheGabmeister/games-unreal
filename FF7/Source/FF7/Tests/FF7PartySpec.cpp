// Copyright Epic Games, Inc. All Rights Reserved.

#include "FF7CharacterDefinition.h"
#include "FF7GameInstance.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FFF7PartySpec, "FF7",
	EAutomationTestFlags::EditorContext
	| EAutomationTestFlags::ClientContext
	| EAutomationTestFlags::ProductFilter)

	UFF7GameInstance* GI = nullptr;
	TArray<UFF7CharacterDefinition*> Defs;

	UFF7CharacterDefinition* MakeDef(const FName& Id, int32 HP)
	{
		UFF7CharacterDefinition* Def = NewObject<UFF7CharacterDefinition>(GI);
		Def->CharacterId = Id;
		Def->DefaultStats.MaxHP = HP;
		Def->DefaultStats.HP = HP;
		Def->DefaultStats.Level = 1;
		return Def;
	}

	void SeedNineMembers()
	{
		const TArray<FName> Names = {
			TEXT("Cloud"),   TEXT("Barret"), TEXT("Tifa"),
			TEXT("Aerith"),  TEXT("RedXIII"),TEXT("Yuffie"),
			TEXT("CaitSith"),TEXT("Vincent"),TEXT("Cid")
		};
		Defs.Reset();
		for (const FName& Name : Names)
		{
			Defs.Add(MakeDef(Name, 100));
		}
		GI->InitializeRoster(Defs);
	}

END_DEFINE_SPEC(FFF7PartySpec)

void FFF7PartySpec::Define()
{
	BeforeEach([this]()
	{
		GI = NewObject<UFF7GameInstance>();
		TestNotNull(TEXT("GameInstance"), GI);
	});

	AfterEach([this]()
	{
		GI = nullptr;
		Defs.Reset();
	});

	Describe("Party", [this]()
	{
		It("InitPopulates9: nine roster entries with matching CharacterIds", [this]()
		{
			SeedNineMembers();
			TestEqual(TEXT("Roster size"), GI->Roster.Num(), 9);
			TestEqual(TEXT("ActiveParty size"), GI->ActivePartyIndices.Num(), 3);
			TestEqual(TEXT("Roster[0] is Cloud"), GI->Roster[0].CharacterId, FName("Cloud"));
			TestEqual(TEXT("Roster[4] is RedXIII"), GI->Roster[4].CharacterId, FName("RedXIII"));
			TestEqual(TEXT("Roster[8] is Cid"), GI->Roster[8].CharacterId, FName("Cid"));
			TestEqual(TEXT("Default actives"), GI->ActivePartyIndices, TArray<int32>{ 0, 1, 2 });
		});

		It("SwapActive: swapping slot 0 with roster idx 3 updates the active party", [this]()
		{
			SeedNineMembers();
			TestTrue(TEXT("Swap OK"), GI->SwapActive(0, 3));
			TestEqual(TEXT("Slot 0 now points at roster[3]"), GI->ActivePartyIndices[0], 3);
			TestEqual(TEXT("Slot 1 unchanged"), GI->ActivePartyIndices[1], 1);
			TestEqual(TEXT("Slot 2 unchanged"), GI->ActivePartyIndices[2], 2);
		});

		It("SwapRejectsDuplicate: can't put an already-active roster idx into another slot", [this]()
		{
			SeedNineMembers();
			// Active is [0,1,2]. Attempt to put roster[1] at slot 0 — already in slot 1.
			TestFalse(TEXT("Duplicate rejected"), GI->SwapActive(0, 1));
			TestEqual(TEXT("Slot 0 unchanged"), GI->ActivePartyIndices[0], 0);
		});

		It("SwapActive: invalid indices return false", [this]()
		{
			SeedNineMembers();
			TestFalse(TEXT("Bad slot"), GI->SwapActive(99, 3));
			TestFalse(TEXT("Bad roster idx"), GI->SwapActive(0, 99));
		});
	});

	Describe("Gil", [this]()
	{
		It("AddSpend: Add increases, Spend decreases, under-spend rejects", [this]()
		{
			TestEqual(TEXT("Initial 0"), GI->Gil, 0);

			GI->AddGil(500);
			TestEqual(TEXT("After +500"), GI->Gil, 500);

			GI->AddGil(-100);  // ignored
			TestEqual(TEXT("Negative ignored"), GI->Gil, 500);

			TestTrue(TEXT("Spend 200 OK"), GI->SpendGil(200));
			TestEqual(TEXT("After -200"), GI->Gil, 300);

			TestFalse(TEXT("Overspend rejected"), GI->SpendGil(10000));
			TestEqual(TEXT("Unchanged"), GI->Gil, 300);
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS
