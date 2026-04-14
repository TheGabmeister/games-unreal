#include "QuakeWeapon_Axe.h"

#include "QuakeBalanceRows.h"
#include "QuakeCharacter.h"
#include "QuakeCollisionChannels.h"
#include "QuakeDamageType_Melee.h"
#include "QuakeSoundManager.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"

AQuakeWeapon_Axe::AQuakeWeapon_Axe()
{
	// SPEC section 2.0: Axe RoF = 2/sec  -> 0.5 s cooldown, no ammo.
	RateOfFire = 2.f;
	AmmoType = EQuakeAmmoType::None;
	AmmoPerShot = 0;
	DisplayName = NSLOCTEXT("QuakeWeapon", "AxeName", "Axe");
	StatsRowName = TEXT("Axe");
}

void AQuakeWeapon_Axe::ApplyStatsFromRow(const FQuakeWeaponStatsRow& Row)
{
	Super::ApplyStatsFromRow(Row);
	Damage = Row.Damage;
	Range = Row.Range;
}

void AQuakeWeapon_Axe::Fire(AActor* InInstigator)
{
	APawn* PawnInstigator = nullptr;
	UWorld* World = nullptr;
	if (!GetFireContext(InInstigator, PawnInstigator, World))
	{
		return;
	}

	// Trace from the pawn's eyes (which on the player is the camera) along
	// the look direction. GetActorEyesViewPoint defers to the camera-facing
	// override on AQuakeCharacter / ACharacter so the swing tracks the look
	// direction, not the actor yaw.
	FVector EyeLoc;
	FRotator EyeRot;
	PawnInstigator->GetActorEyesViewPoint(EyeLoc, EyeRot);

	const FVector TraceStart = EyeLoc;
	const FVector ShotDir = EyeRot.Vector();
	const FVector TraceEnd = TraceStart + ShotDir * Range;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(QuakeAxeMelee), /*bTraceComplex*/ false, this);
	Params.AddIgnoredActor(InInstigator);
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		QuakeCollision::ECC_Weapon,
		Params);

	// Noise event for AI hearing. SPEC section 3.3: firing any weapon (and
	// taking damage) reports a noise event via UAISense_Hearing; the Grunt's
	// sense config has bUseLoSHearing=false, so the swing is audible through
	// walls just like original Quake. Source location is the swing origin
	// (the player's eyes), loudness 1.0 — hearing range is determined per-
	// enemy on the AISenseConfig_Hearing side.
	UAISense_Hearing::ReportNoiseEvent(
		World,
		TraceStart,
		/*Loudness*/ 1.f,
		InInstigator,
		/*MaxRange*/ 0.f,
		FName(TEXT("QuakeWeaponFire")));

	UQuakeSoundManager::PlaySoundEvent(this, EQuakeSoundEvent::WeaponAxeSwing, TraceStart);

#if !UE_BUILD_SHIPPING
	// Dev visualization: red miss, green hit. 1-second lifetime so single
	// swings are easy to see in PIE. Stripped from shipping builds.
	const FColor LineColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugLine(World, TraceStart, TraceEnd, LineColor, /*bPersistent*/ false, /*Lifetime*/ 1.f, 0, /*Thickness*/ 1.f);
#endif

	if (!bHit || !Hit.GetActor())
	{
		return;
	}

	// Damage attribution: EventInstigator is the firing pawn's controller,
	// DamageCauser is the weapon. This matches SPEC section 1.5 — TakeDamage
	// implementations infer self-damage and infighting attribution from
	// these standard parameters.
	AController* InstigatorController = PawnInstigator->GetController();

	// SPEC 4.3: Quad = 4× outgoing weapon damage. Read off the firer so
	// enemy axe-equivalents (Knight melee) don't inherit the player's Quad.
	float ScaledDamage = Damage;
	if (const AQuakeCharacter* QuakePawn = Cast<AQuakeCharacter>(PawnInstigator))
	{
		ScaledDamage *= QuakePawn->GetOutgoingDamageScale();
	}

	UGameplayStatics::ApplyPointDamage(
		Hit.GetActor(),
		ScaledDamage,
		ShotDir,
		Hit,
		InstigatorController,
		this,
		UQuakeDamageType_Melee::StaticClass());
}
