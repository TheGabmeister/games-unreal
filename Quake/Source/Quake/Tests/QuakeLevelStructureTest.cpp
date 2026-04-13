// Phase 8 unit tests.
//
// Covers the SPEC 1.5 damage-type metadata for the two new damage types
// introduced in Phase 8 (Lava, Telefrag). Same shared-base CDO cast pattern
// as QuakeCharacterDamageTest.cpp — resolves a leaf damage-type class back
// to the abstract UQuakeDamageType base and asserts the flag defaults, so a
// future constructor regression trips the test before it ships.
//
// The SPEC Phase 8 "Functional test: AQuakeTrigger_Relay with 3 mock
// targets" is deferred to the manual LevelStructureSandbox walkthrough —
// it requires spawning AActors in a world, which the project's unit-test
// pattern (world-free) deliberately avoids. The interface dispatch itself
// is trivial (TArray iterate + Cast<IQuakeActivatable> + Activate call);
// the sandbox map is the authoritative regression for that path.

#include "Misc/AutomationTest.h"

#include "QuakeDamageType.h"
#include "QuakeDamageType_Lava.h"
#include "QuakeDamageType_Telefrag.h"

#include "Engine/DamageEvents.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr float kDamageTypeTolerance = UE_KINDA_SMALL_NUMBER;

	// Reusable helper: resolve a leaf damage-type class to its abstract base
	// via the SPEC section 1.5 cast pattern. Returns nullptr if unresolvable.
	const UQuakeDamageType* ResolveBaseCDO(TSubclassOf<UDamageType> LeafClass)
	{
		FDamageEvent DamageEvent;
		DamageEvent.DamageTypeClass = LeafClass;
		return Cast<UQuakeDamageType>(
			DamageEvent.DamageTypeClass
				? DamageEvent.DamageTypeClass->GetDefaultObject()
				: UQuakeDamageType::StaticClass()->GetDefaultObject());
	}
}

// -----------------------------------------------------------------------------
// SPEC 1.5 row for UQuakeDamageType_Lava:
//   bSuppressesPain = true, bCausedByWorld = true.
// Other flags inherit base defaults.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeDamageTypeLavaTest,
	"Quake.Damage.DamageType.Lava",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeDamageTypeLavaTest::RunTest(const FString& /*Parameters*/)
{
	const UQuakeDamageType* DT = ResolveBaseCDO(UQuakeDamageType_Lava::StaticClass());
	TestNotNull(TEXT("Lava resolves to UQuakeDamageType base"), DT);
	if (!DT)
	{
		return false;
	}

	TestTrue (TEXT("Lava bSuppressesPain = true"),  DT->bSuppressesPain);
	TestTrue (TEXT("Lava bCausedByWorld = true"),   DT->bCausedByWorld);
	TestFalse(TEXT("Lava bIgnoresArmor default"),   DT->bIgnoresArmor);
	TestFalse(TEXT("Lava bBypassesBiosuit default"),DT->bBypassesBiosuit);
	TestTrue (TEXT("Lava bSelfDamage default"),     DT->bSelfDamage);
	TestEqual(TEXT("Lava SelfDamageScale default"), DT->SelfDamageScale, 1.0f, kDamageTypeTolerance);
	TestEqual(TEXT("Lava KnockbackScale default"),  DT->KnockbackScale,  1.0f, kDamageTypeTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// SPEC 1.5 row for UQuakeDamageType_Telefrag:
//   bSuppressesPain = true. Damage magnitude (10000) is caller-owned.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeDamageTypeTelefragTest,
	"Quake.Damage.DamageType.Telefrag",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeDamageTypeTelefragTest::RunTest(const FString& /*Parameters*/)
{
	const UQuakeDamageType* DT = ResolveBaseCDO(UQuakeDamageType_Telefrag::StaticClass());
	TestNotNull(TEXT("Telefrag resolves to UQuakeDamageType base"), DT);
	if (!DT)
	{
		return false;
	}

	TestTrue (TEXT("Telefrag bSuppressesPain = true"),  DT->bSuppressesPain);
	TestFalse(TEXT("Telefrag bCausedByWorld default"),  DT->bCausedByWorld);
	TestFalse(TEXT("Telefrag bIgnoresArmor default"),   DT->bIgnoresArmor);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
