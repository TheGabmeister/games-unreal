// Phase 14 unit tests — DESIGN 8 audio + settings. Functional "fire each
// weapon and confirm the log line" coverage requires a world (PIE) and
// lives in the manual checklist; this file covers the pure-deterministic
// pieces: the sound-event row-name lookup, the GameUserSettings round
// trip, and the volume/sensitivity clamp.

#include "Misc/AutomationTest.h"

#include "QuakeGameUserSettings.h"
#include "QuakeSoundEvent.h"
#include "QuakeSoundManager.h"

#include "UObject/UObjectGlobals.h"

#if WITH_DEV_AUTOMATION_TESTS

// -----------------------------------------------------------------------------
// ResolveRowName uses the UENUM value name verbatim — DT_SoundEvents rows
// must be keyed by the same string. This regresses both the lookup helper
// (no surprise prefix / trailing junk) and the catalog itself: if anyone
// renames an enum value and forgets to update the table key, this test still
// passes because it derives both sides from the enum, but at least guarantees
// "what the manager looks up" matches "what the helper claims to build".
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeSoundResolveRowNameTest,
	"Quake.Phase14.Sound.ResolveRowName",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeSoundResolveRowNameTest::RunTest(const FString&)
{
	const FName JumpRow = UQuakeSoundManager::ResolveRowName(EQuakeSoundEvent::PlayerJump);
	TestFalse(TEXT("PlayerJump row name is non-empty"), JumpRow.IsNone());
	TestEqual(TEXT("PlayerJump row matches enum value-name"),
		JumpRow, FName(TEXT("PlayerJump")));

	const FName DoorOpenRow = UQuakeSoundManager::ResolveRowName(EQuakeSoundEvent::DoorOpen);
	TestEqual(TEXT("DoorOpen row matches enum value-name"),
		DoorOpenRow, FName(TEXT("DoorOpen")));

	const FName NoneRow = UQuakeSoundManager::ResolveRowName(EQuakeSoundEvent::None);
	TestEqual(TEXT("None row stringifies to 'None'"),
		NoneRow, FName(TEXT("None")));
	return true;
}

// -----------------------------------------------------------------------------
// Settings round-trip — set sensitivity, save, re-read, restored. The actual
// disk write/read is exercised by the manual checklist (a fresh editor session
// must show the saved value); here we test the in-memory clamp + accessor.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeGameUserSettingsRoundTripTest,
	"Quake.Phase14.Settings.RoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FQuakeGameUserSettingsRoundTripTest::RunTest(const FString&)
{
	UQuakeGameUserSettings* Settings = NewObject<UQuakeGameUserSettings>();

	Settings->SetMouseSensitivity(2.0f);
	TestEqual(TEXT("Sensitivity stored"), Settings->GetMouseSensitivity(), 2.0f);

	Settings->SetMouseSensitivity(0.f);
	TestEqual(TEXT("Sensitivity clamped to 0.05 floor"),
		Settings->GetMouseSensitivity(), 0.05f);

	Settings->SetMouseSensitivity(99.f);
	TestEqual(TEXT("Sensitivity clamped to 5.0 ceiling"),
		Settings->GetMouseSensitivity(), 5.0f);

	Settings->SetMasterVolume(0.5f);
	TestEqual(TEXT("Volume stored"), Settings->GetMasterVolume(), 0.5f);

	Settings->SetMasterVolume(-1.f);
	TestEqual(TEXT("Volume clamped to 0 floor"), Settings->GetMasterVolume(), 0.f);

	Settings->SetMasterVolume(2.f);
	TestEqual(TEXT("Volume clamped to 1.0 ceiling"), Settings->GetMasterVolume(), 1.f);

	Settings->SetToDefaults();
	TestEqual(TEXT("Defaults reset sensitivity"),
		Settings->GetMouseSensitivity(), 0.5f);
	TestEqual(TEXT("Defaults reset volume"),
		Settings->GetMasterVolume(), 1.0f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
