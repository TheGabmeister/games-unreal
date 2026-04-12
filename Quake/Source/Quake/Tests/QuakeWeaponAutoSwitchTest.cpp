// Unit tests for the Phase 6 SPEC 2.2 empty-ammo auto-switch priority
// picker AQuakeCharacter::PickAutoSwitchWeaponSlot.
//
// The SPEC 2.2 priority order is RL -> SNG -> SSG -> NG -> SG -> Axe.
// Thunderbolt (slot index 7) and GL (slot index 5) are deliberately NOT in
// the list — they are "kept manual to avoid accidental switching".
//
// The helper is a pure static function that takes two parallel length-8
// bool arrays (ownership mask + ammo mask) + an exclude slot, and returns
// the best slot index or -1. No world, no character, no GameInstance —
// same pattern as ApplyArmorAbsorption, ApplyQuakeAirAccel,
// ComputePainChance, and ComputeLinearFalloffDamage.

#include "Misc/AutomationTest.h"

#include "QuakeCharacter.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	// Fixture helper: build a length-8 bool array with the specified slot
	// indices set to true and everything else false. Keeps each test's
	// setup readable.
	static TArray<bool> MakeMask(std::initializer_list<int32> TrueSlots)
	{
		TArray<bool> Mask;
		Mask.Init(false, AQuakeCharacter::NumWeaponSlots);
		for (const int32 Slot : TrueSlots)
		{
			if (Slot >= 0 && Slot < Mask.Num())
			{
				Mask[Slot] = true;
			}
		}
		return Mask;
	}
}

// -----------------------------------------------------------------------------
// Nothing eligible: no weapons owned, no ammo anywhere. Returns -1 so the
// caller knows to leave the current (click-only) weapon equipped instead
// of forcing a no-op switch.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAutoSwitchNoEligibleTest,
	"Quake.Weapon.AutoSwitch.NoEligible",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAutoSwitchNoEligibleTest::RunTest(const FString& /*Parameters*/)
{
	const TArray<bool> Owned   = MakeMask({});
	const TArray<bool> HasAmmo = MakeMask({});
	const int32 Best = AQuakeCharacter::PickAutoSwitchWeaponSlot(Owned, HasAmmo, /*Exclude*/ 1);
	TestEqual(TEXT("No eligible slot returns -1"), Best, -1);
	return true;
}

// -----------------------------------------------------------------------------
// Axe is the priority fallback. When the shotgun (slot 1) fires empty and
// only the Axe (slot 0) is owned, the picker returns 0.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAutoSwitchAxeFallbackTest,
	"Quake.Weapon.AutoSwitch.AxeFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAutoSwitchAxeFallbackTest::RunTest(const FString& /*Parameters*/)
{
	const TArray<bool> Owned   = MakeMask({ 0, 1 });  // Axe + Shotgun
	const TArray<bool> HasAmmo = MakeMask({ 0 });      // Axe yes, Shotgun no
	const int32 Best = AQuakeCharacter::PickAutoSwitchWeaponSlot(Owned, HasAmmo, /*Exclude*/ 1);
	TestEqual(TEXT("Falls back to Axe (slot 0)"), Best, 0);
	return true;
}

// -----------------------------------------------------------------------------
// Priority order check — Rocket Launcher is top. Shotgun is current (slot
// 1) and empty; all four v1 weapons are owned and have ammo; picker
// returns slot 6 (Rocket Launcher), not slot 0 (Axe) or 3 (Nailgun).
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAutoSwitchRocketTopPriorityTest,
	"Quake.Weapon.AutoSwitch.RocketTopPriority",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAutoSwitchRocketTopPriorityTest::RunTest(const FString& /*Parameters*/)
{
	const TArray<bool> Owned   = MakeMask({ 0, 1, 3, 6 });
	const TArray<bool> HasAmmo = MakeMask({ 0, 1, 3, 6 });
	const int32 Best = AQuakeCharacter::PickAutoSwitchWeaponSlot(Owned, HasAmmo, /*Exclude*/ 1);
	TestEqual(TEXT("RL is top priority"), Best, 6);
	return true;
}

// -----------------------------------------------------------------------------
// Priority order check — Nailgun beats Shotgun when RL has no rockets.
// Shotgun is current (slot 1) and empty; Axe / Shotgun / Nailgun / Rocket
// Launcher all owned, RL out of rockets, Nailgun has nails. Picker
// returns 3 (Nailgun), not 0 (Axe).
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAutoSwitchNailgunBeatsAxeTest,
	"Quake.Weapon.AutoSwitch.NailgunBeatsAxe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAutoSwitchNailgunBeatsAxeTest::RunTest(const FString& /*Parameters*/)
{
	const TArray<bool> Owned   = MakeMask({ 0, 1, 3, 6 });
	const TArray<bool> HasAmmo = MakeMask({ 0, 3 });  // Axe yes, Nails yes, SG+RL no
	const int32 Best = AQuakeCharacter::PickAutoSwitchWeaponSlot(Owned, HasAmmo, /*Exclude*/ 1);
	TestEqual(TEXT("Nailgun (3) beats Axe (0)"), Best, 3);
	return true;
}

// -----------------------------------------------------------------------------
// Exclude slot: if the Rocket Launcher is the current weapon and fires
// empty, the picker must not return 6 even though RL is top priority.
// It should fall through to the next eligible slot (Nailgun here).
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAutoSwitchExcludesCurrentTest,
	"Quake.Weapon.AutoSwitch.ExcludesCurrent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAutoSwitchExcludesCurrentTest::RunTest(const FString& /*Parameters*/)
{
	// Everything owned, everything has ammo, but RL is current (excluded).
	const TArray<bool> Owned   = MakeMask({ 0, 1, 3, 6 });
	const TArray<bool> HasAmmo = MakeMask({ 0, 1, 3, 6 });
	const int32 Best = AQuakeCharacter::PickAutoSwitchWeaponSlot(Owned, HasAmmo, /*Exclude*/ 6);
	TestEqual(TEXT("Skips the current slot"), Best, 3);  // Nailgun (next in priority)
	return true;
}

// -----------------------------------------------------------------------------
// GL (slot 5) and Thunderbolt (slot 7) must NOT be auto-switch targets.
// Populate only those two slots + set them to have ammo — picker must
// still return -1 because neither is in the priority list.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAutoSwitchGLAndThunderboltSkippedTest,
	"Quake.Weapon.AutoSwitch.GLAndThunderboltSkipped",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAutoSwitchGLAndThunderboltSkippedTest::RunTest(const FString& /*Parameters*/)
{
	const TArray<bool> Owned   = MakeMask({ 5, 7 });
	const TArray<bool> HasAmmo = MakeMask({ 5, 7 });
	const int32 Best = AQuakeCharacter::PickAutoSwitchWeaponSlot(Owned, HasAmmo, /*Exclude*/ -1);
	TestEqual(TEXT("GL and Thunderbolt are never auto-switch targets"), Best, -1);
	return true;
}

// -----------------------------------------------------------------------------
// Owned but no ammo: a weapon that is owned but empty must be skipped even
// if it's higher in priority than another eligible slot. Axe (slot 0)
// always has ammo (AmmoType::None) so it's the terminal fallback when
// nothing above it has usable ammo.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeAutoSwitchOwnedButEmptySkippedTest,
	"Quake.Weapon.AutoSwitch.OwnedButEmptySkipped",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeAutoSwitchOwnedButEmptySkippedTest::RunTest(const FString& /*Parameters*/)
{
	const TArray<bool> Owned   = MakeMask({ 0, 1, 3, 6 });  // all four v1 weapons owned
	const TArray<bool> HasAmmo = MakeMask({ 0 });            // but only Axe has "ammo"
	const int32 Best = AQuakeCharacter::PickAutoSwitchWeaponSlot(Owned, HasAmmo, /*Exclude*/ 3);
	TestEqual(TEXT("Empty owned weapons are skipped"), Best, 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
