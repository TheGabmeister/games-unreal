// Unit tests for the Phase 2 damage pipeline:
//   1. AQuakeCharacter::ApplyArmorAbsorption — pure formula, no world.
//   2. The shared-base CDO cast pattern from SPEC section 1.5 — verify a
//      damage event constructed with a leaf damage type class resolves
//      back to the abstract UQuakeDamageType base via the standard cast,
//      and that the field defaults read uniformly without per-leaf
//      branching.
//
// Per CLAUDE.md "Running Tests": tests are EditorContext + EngineFilter,
// guarded by WITH_DEV_AUTOMATION_TESTS, and live under Source/Quake/Tests.
// Run from Session Frontend -> Automation tab with filter "Quake.*".
//
// LWC reminder: numeric assertions on FVector components must use double
// literals — that's why every TestEqual uses 0.0 / 30.0 / etc. and
// UE_KINDA_SMALL_NUMBER instead of KINDA_SMALL_NUMBER.

#include "Misc/AutomationTest.h"

#include "QuakeCharacter.h"
#include "QuakeDamageType.h"
#include "QuakeDamageType_Melee.h"

#include "Engine/DamageEvents.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr float kArmorTolerance = UE_KINDA_SMALL_NUMBER;
}

// -----------------------------------------------------------------------------
// SPEC Phase 2 unit test:
//     100 HP + 100 green armor (absorption=0.3), take 50 damage.
//     Quake formula: save = ceil(0.3 * 50) = 15, take = 35.
//     Result: HP = 100-35 = 65, Armor = 100-15 = 85.
//
// Note: an earlier draft of SPEC section 11.5 listed "HP = 85, armor = 65"
// for this test, which had the HP and armor results swapped. The numbers
// here follow SPEC section 1.2's prose ("Green Armor absorbs 30% of
// damage") and the original Quake T_Damage formula. SPEC line 1443 was
// updated alongside this test to match.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeArmorAbsorptionGreen50Test,
	"Quake.Damage.ArmorAbsorption.Green50",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeArmorAbsorptionGreen50Test::RunTest(const FString& /*Parameters*/)
{
	float OutHealth = 0.f;
	float OutArmor = 0.f;
	AQuakeCharacter::ApplyArmorAbsorption(
		/*InHealth*/ 100.f,
		/*InArmor*/  100.f,
		/*InAbsorption*/ 0.3f,
		/*InDamage*/ 50.f,
		OutHealth, OutArmor);

	TestEqual(TEXT("HP loses 35 (= 50 - ceil(0.3*50))"), OutHealth, 65.f, kArmorTolerance);
	TestEqual(TEXT("Armor loses 15 (= ceil(0.3*50))"),   OutArmor,  85.f, kArmorTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Yellow armor variant: 60% absorption.
// 50 damage * 0.6 = 30 -> save=30, take=20. HP=80, Armor=120.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeArmorAbsorptionYellow50Test,
	"Quake.Damage.ArmorAbsorption.Yellow50",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeArmorAbsorptionYellow50Test::RunTest(const FString& /*Parameters*/)
{
	float OutHealth = 0.f;
	float OutArmor = 0.f;
	AQuakeCharacter::ApplyArmorAbsorption(100.f, 150.f, 0.6f, 50.f, OutHealth, OutArmor);

	TestEqual(TEXT("HP loses 20"),     OutHealth, 80.f,  kArmorTolerance);
	TestEqual(TEXT("Armor loses 30"),  OutArmor,  120.f, kArmorTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// Armor underflow: armor doesn't absorb more than it has, the rest spills
// to HP. 100 HP + 10 green armor + 50 damage:
//     save = ceil(0.3 * 50) = 15
//     save > armor (10), so save = 10
//     take = 50 - 10 = 40
//     -> HP = 60, Armor = 0
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeArmorAbsorptionUnderflowTest,
	"Quake.Damage.ArmorAbsorption.Underflow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeArmorAbsorptionUnderflowTest::RunTest(const FString& /*Parameters*/)
{
	float OutHealth = 0.f;
	float OutArmor = 0.f;
	AQuakeCharacter::ApplyArmorAbsorption(100.f, 10.f, 0.3f, 50.f, OutHealth, OutArmor);

	TestEqual(TEXT("Armor depleted"),         OutArmor,  0.f,  kArmorTolerance);
	TestEqual(TEXT("HP takes the overflow"),  OutHealth, 60.f, kArmorTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// No armor at all: absorption is zero, damage flows straight to HP.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeArmorAbsorptionNoArmorTest,
	"Quake.Damage.ArmorAbsorption.NoArmor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeArmorAbsorptionNoArmorTest::RunTest(const FString& /*Parameters*/)
{
	float OutHealth = 0.f;
	float OutArmor = 0.f;
	AQuakeCharacter::ApplyArmorAbsorption(100.f, 0.f, 0.f, 25.f, OutHealth, OutArmor);

	TestEqual(TEXT("HP loses full damage"),  OutHealth, 75.f, kArmorTolerance);
	TestEqual(TEXT("Armor stays at zero"),   OutArmor,  0.f,  kArmorTolerance);
	return true;
}

// -----------------------------------------------------------------------------
// SPEC Phase 2 unit test: shared-base CDO cast.
// Construct an FDamageEvent with UQuakeDamageType_Melee::StaticClass(),
// run the cast pattern from SPEC section 1.5, and assert the resulting
// UQuakeDamageType* is non-null and has the expected default field values.
//
// The point of this test is regression: if someone accidentally adds a
// UPROPERTY override in UQuakeDamageType_Melee that flips one of the base
// flags, this asserts the leaf is still readable as the abstract base
// without any RTTI per-leaf branching.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeDamageTypeCDOCastTest,
	"Quake.Damage.DamageType.SharedBaseCDOCast",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeDamageTypeCDOCastTest::RunTest(const FString& /*Parameters*/)
{
	// Build the same kind of FDamageEvent that ApplyPointDamage constructs
	// internally. We don't need a hit result or shot direction for the
	// cast itself — the only field that matters is DamageTypeClass.
	FDamageEvent DamageEvent;
	DamageEvent.DamageTypeClass = UQuakeDamageType_Melee::StaticClass();

	const UQuakeDamageType* DT = Cast<UQuakeDamageType>(
		DamageEvent.DamageTypeClass
			? DamageEvent.DamageTypeClass->GetDefaultObject()
			: UQuakeDamageType::StaticClass()->GetDefaultObject());

	TestNotNull(TEXT("Cast resolves to UQuakeDamageType base"), DT);
	if (!DT)
	{
		return false;
	}

	// Melee uses every base default unchanged. If a future change adds an
	// override in UQuakeDamageType_Melee, update this test deliberately —
	// don't paper over the failure.
	TestFalse(TEXT("bIgnoresArmor default false"),     DT->bIgnoresArmor);
	TestFalse(TEXT("bSuppressesPain default false"),   DT->bSuppressesPain);
	TestFalse(TEXT("bBypassesBiosuit default false"),  DT->bBypassesBiosuit);
	TestTrue (TEXT("bSelfDamage default true"),        DT->bSelfDamage);
	TestEqual(TEXT("SelfDamageScale default 1.0"),     DT->SelfDamageScale, 1.0f, kArmorTolerance);
	TestEqual(TEXT("KnockbackScale default 1.0"),      DT->KnockbackScale,  1.0f, kArmorTolerance);

	return true;
}

// -----------------------------------------------------------------------------
// Same cast pattern but with a null DamageTypeClass — the SPEC fallback to
// UQuakeDamageType::StaticClass()->GetDefaultObject() should still produce
// a non-null base CDO, so call sites that omit a damage type entirely
// don't have to special-case the null check.
// -----------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuakeDamageTypeCDOCastNullFallbackTest,
	"Quake.Damage.DamageType.SharedBaseCDOCastNullFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuakeDamageTypeCDOCastNullFallbackTest::RunTest(const FString& /*Parameters*/)
{
	FDamageEvent DamageEvent;
	DamageEvent.DamageTypeClass = nullptr;

	const UQuakeDamageType* DT = Cast<UQuakeDamageType>(
		DamageEvent.DamageTypeClass
			? DamageEvent.DamageTypeClass->GetDefaultObject()
			: UQuakeDamageType::StaticClass()->GetDefaultObject());

	TestNotNull(TEXT("Null DamageTypeClass falls back to abstract base CDO"), DT);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
