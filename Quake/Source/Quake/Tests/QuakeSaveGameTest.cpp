// Phase 11 unit tests — save archive round-trip, F5 gate predicate,
// consumed-pickup set math, time-base translation.
//
// Per CLAUDE.md "Prefer pure static helpers over world-spinup tests",
// these exercise the formulas and predicates directly. Full round-trip
// through UQuakeGameInstance::SaveCurrentState / LoadFromSlot, pawn
// teleport, and IQuakeSaveable actor iteration lives in the Phase 11
// Manual verification checklist.

#include "Misc/AutomationTest.h"

#include "QuakeSaveArchive.h"
#include "QuakeSaveGame.h"
#include "QuakeSaveable.h"

#include "GameFramework/CharacterMovementComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

// -----------------------------------------------------------------------------
// DESIGN 6.2: F5 is accepted iff the player is grounded, not flinching,
// and not dead. Matrix over the relevant movement modes.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeSaveCanQuickSaveMatrixTest,
	"Quake.Save.CanQuickSave.Matrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeSaveCanQuickSaveMatrixTest::RunTest(const FString&)
{
	using namespace QuakeSaveArchive;

	TestTrue (TEXT("Walking, no pain, alive -> accept"),     CanQuickSave(MOVE_Walking, false, false));
	TestFalse(TEXT("Walking, in pain, alive -> reject"),     CanQuickSave(MOVE_Walking, true,  false));
	TestFalse(TEXT("Walking, no pain, dead -> reject"),      CanQuickSave(MOVE_Walking, false, true));
	TestFalse(TEXT("Falling -> reject"),                     CanQuickSave(MOVE_Falling, false, false));
	TestFalse(TEXT("Swimming -> reject"),                    CanQuickSave(MOVE_Swimming, false, false));
	TestFalse(TEXT("Flying -> reject"),                      CanQuickSave(MOVE_Flying,  false, false));
	TestFalse(TEXT("None -> reject"),                        CanQuickSave(MOVE_None,    false, false));

	return true;
}

// -----------------------------------------------------------------------------
// Consumed-pickup set difference. Pickups currently live = not consumed.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeSaveConsumedPickupSetTest,
	"Quake.Save.ConsumedPickupNames.SetDifference",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeSaveConsumedPickupSetTest::RunTest(const FString&)
{
	using namespace QuakeSaveArchive;

	const FName A(TEXT("Pickup_Health_0"));
	const FName B(TEXT("Pickup_Ammo_0"));
	const FName C(TEXT("Pickup_Armor_0"));

	// Nothing consumed.
	{
		const TArray<FName> Consumed = ComputeConsumedNames({A, B, C}, {A, B, C});
		TestEqual(TEXT("All live -> zero consumed"), Consumed.Num(), 0);
	}
	// Middle one consumed.
	{
		const TArray<FName> Consumed = ComputeConsumedNames({A, B, C}, {A, C});
		TestEqual(TEXT("One missing -> one consumed"), Consumed.Num(), 1);
		TestTrue (TEXT("Consumed contains B"),         Consumed.Contains(B));
	}
	// Everything consumed.
	{
		const TArray<FName> Consumed = ComputeConsumedNames({A, B, C}, {});
		TestEqual(TEXT("All missing -> three consumed"), Consumed.Num(), 3);
	}
	// Empty initial (fresh level, nothing to compare).
	{
		const TArray<FName> Consumed = ComputeConsumedNames({}, {A});
		TestEqual(TEXT("Empty initial -> empty consumed"), Consumed.Num(), 0);
	}
	return true;
}

// -----------------------------------------------------------------------------
// DESIGN 6.2: PlayerState::GetTimeElapsed() = WorldTime - LevelStartTime.
// On load we rewind LevelStartTime so the elapsed count resumes at the saved
// value.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeSaveTimeBaseTranslationTest,
	"Quake.Save.TimeBase.RestoreLevelStartTime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeSaveTimeBaseTranslationTest::RunTest(const FString&)
{
	using namespace QuakeSaveArchive;

	// Saved 12.5 s into a level; world time on load happens to be 100 s.
	const double WorldNow = 100.0;
	const float ElapsedAtSave = 12.5f;
	const double Restored = ComputeRestoredLevelStartTime(WorldNow, ElapsedAtSave);

	TestEqual(TEXT("Restored LevelStartTime = WorldNow - ElapsedAtSave"),
		Restored, 87.5, static_cast<double>(UE_KINDA_SMALL_NUMBER));

	// Re-computing elapsed from the restored base yields the saved value.
	const double ReElapsed = WorldNow - Restored;
	TestEqual(TEXT("Elapsed round-trips"),
		ReElapsed, static_cast<double>(ElapsedAtSave), static_cast<double>(UE_KINDA_SMALL_NUMBER));

	return true;
}

// -----------------------------------------------------------------------------
// FActorSaveRecord stores its Payload as raw bytes. Confirm the POD layer
// round-trips; actor-level serialization through FObjectAndNameAsStringProxy
// is covered by the Manual checklist.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeSaveActorRecordRoundTripTest,
	"Quake.Save.ActorRecord.PayloadRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeSaveActorRecordRoundTripTest::RunTest(const FString&)
{
	FActorSaveRecord Rec;
	Rec.ActorName = FName(TEXT("QuakeDoor_42"));
	Rec.Payload   = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01, 0x02};

	TestEqual(TEXT("ActorName survives"), Rec.ActorName, FName(TEXT("QuakeDoor_42")));
	TestEqual(TEXT("Payload length"),     Rec.Payload.Num(), 7);
	TestEqual(TEXT("Payload byte 0"),     Rec.Payload[0], static_cast<uint8>(0xDE));
	TestEqual(TEXT("Payload byte 3"),     Rec.Payload[3], static_cast<uint8>(0xEF));
	return true;
}

// -----------------------------------------------------------------------------
// UQuakeSaveGame is a POD-ish container of UPROPERTYs — confirm scalar
// fields retain their assigned values after NewObject. (Serialization to
// disk is wired via UGameplayStatics and exercised only in PIE.)
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeSaveGameFieldsRetainTest,
	"Quake.Save.SaveGame.FieldsRetain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeSaveGameFieldsRetainTest::RunTest(const FString&)
{
	UQuakeSaveGame* Save = NewObject<UQuakeSaveGame>();
	Save->ProfileName      = TEXT("default");
	Save->Health           = 73.5f;
	Save->CurrentLevelName = TEXT("E1M1");
	Save->Kills            = 12;
	Save->Secrets          = 2;
	Save->Deaths           = 1;
	Save->ElapsedAtSave    = 145.2f;

	// Inventory is now nested under FQuakeInventorySnapshot — matches the
	// component's Serialize/Deserialize round-trip shape.
	Save->InventorySnapshot.Armor           = 100.f;
	Save->InventorySnapshot.ArmorAbsorption = 0.3f;
	Save->InventorySnapshot.AmmoCounts.Add(EQuakeAmmoType::Shells,  25);
	Save->InventorySnapshot.AmmoCounts.Add(EQuakeAmmoType::Rockets, 8);
	Save->InventorySnapshot.bValid = true;
	Save->Keys.Add(EQuakeKeyColor::Silver);

	TestEqual(TEXT("ProfileName"),  Save->ProfileName,      FString(TEXT("default")));
	TestEqual(TEXT("Armor"),        Save->InventorySnapshot.Armor,           100.f);
	TestEqual(TEXT("Absorption"),   Save->InventorySnapshot.ArmorAbsorption, 0.3f);
	TestEqual(TEXT("Health"),       Save->Health,           73.5f);
	TestEqual(TEXT("Level"),        Save->CurrentLevelName, FString(TEXT("E1M1")));
	TestEqual(TEXT("Kills"),        Save->Kills,            12);
	TestEqual(TEXT("Shells"),       Save->InventorySnapshot.AmmoCounts.FindRef(EQuakeAmmoType::Shells),  25);
	TestEqual(TEXT("Rockets"),      Save->InventorySnapshot.AmmoCounts.FindRef(EQuakeAmmoType::Rockets), 8);
	TestTrue (TEXT("Silver key"),   Save->Keys.Contains(EQuakeKeyColor::Silver));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
