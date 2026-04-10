#include "QuakeWeapon_Axe.h"

#include "QuakeCollisionChannels.h"
#include "QuakeDamageType_Melee.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeWeapon, Log, All);

AQuakeWeapon_Axe::AQuakeWeapon_Axe()
{
	// SPEC section 2.0: Axe RoF = 2/sec  -> 0.5 s cooldown.
	RateOfFire = 2.f;
}

void AQuakeWeapon_Axe::Fire(AActor* InInstigator)
{
	APawn* PawnInstigator = Cast<APawn>(InInstigator);
	if (!PawnInstigator)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
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

#if !UE_BUILD_SHIPPING
	// Phase 2 dev visualization: red miss, green hit. The line stays visible
	// for 1 second so a single swing is easy to see in PIE. Stripped from
	// shipping builds.
	const FColor LineColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugLine(World, TraceStart, TraceEnd, LineColor, /*bPersistent*/ false, /*Lifetime*/ 1.f, 0, /*Thickness*/ 1.f);
	UE_LOG(LogQuakeWeapon, Log, TEXT("Axe::Fire trace start=%s end=%s hit=%s actor=%s"),
		*TraceStart.ToString(), *TraceEnd.ToString(),
		bHit ? TEXT("YES") : TEXT("NO"),
		Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("none"));
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

	UGameplayStatics::ApplyPointDamage(
		Hit.GetActor(),
		Damage,
		ShotDir,
		Hit,
		InstigatorController,
		this,
		UQuakeDamageType_Melee::StaticClass());
}
