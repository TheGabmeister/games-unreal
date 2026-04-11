#include "QuakeEnemyBase.h"

#include "QuakeCollisionChannels.h"
#include "QuakeDamageType.h"
#include "QuakeEnemyAIController.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeEnemy, Log, All);

AQuakeEnemyBase::AQuakeEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Match the player's capsule dimensions so the shared navmesh agent
	// (radius 35 / height 180) still pathfinds correctly. SPEC section 1.6.
	GetCapsuleComponent()->InitCapsuleSize(35.f, 90.f);

	// SPEC section 1.6 object response matrix for an enemy capsule (Pawn
	// channel): block WorldStatic/WorldDynamic/Pawn/Projectile/Weapon/
	// Visibility; ignore Pickup and Corpse. The engine's ECR_Block default
	// covers most of this — we only need to explicitly carve out Pickup and
	// Corpse. Post-death, OnCorpseChannelFlip swaps the object type to
	// Corpse and blanks everything except WorldStatic.
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	Capsule->SetCollisionObjectType(ECC_Pawn);
	Capsule->SetCollisionResponseToChannel(QuakeCollision::ECC_Pickup, ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(QuakeCollision::ECC_Corpse, ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(QuakeCollision::ECC_Projectile, ECR_Block);
	Capsule->SetCollisionResponseToChannel(QuakeCollision::ECC_Weapon, ECR_Block);

	// Primitive mesh components. BP subclasses fill in the StaticMesh /
	// MaterialInstance slots — the C++ base only owns the component hierarchy
	// and positions so placement is consistent across subclasses.
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));

	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(BodyMesh);
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 170.f));

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(BodyMesh);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetRelativeLocation(FVector(30.f, 20.f, 120.f));

	// Stock UE CharacterMovementComponent is fine for enemies — they don't
	// need the Quake strafe-jump dot-product clamp. MaxWalkSpeed is wired to
	// the per-enemy WalkSpeed property in BeginPlay.
}

void AQuakeEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	// Apply per-enemy walk speed to the stock CMC. Doing this in BeginPlay
	// (rather than the constructor) lets subclasses change WalkSpeed without
	// needing to duplicate the CMC write.
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = WalkSpeed;
	}
}

float AQuakeEnemyBase::ComputePainChance(float Damage, float InMaxHealth)
{
	if (InMaxHealth <= 0.f || Damage <= 0.f)
	{
		return 0.f;
	}
	return FMath::Min(0.8f, (Damage / InMaxHealth) * 2.f);
}

void AQuakeEnemyBase::MoveToTarget(const FVector& TargetLocation)
{
	// Issued through the AAIController so the path-following component and
	// navmesh query run correctly. Not via UCharacterMovementComponent
	// directly — the AIController is the canonical MoveTo entry point.
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->MoveToLocation(TargetLocation, /*AcceptanceRadius*/ 50.f);
	}
}

void AQuakeEnemyBase::PlayPainReaction()
{
	// Phase 3 stub — real pain reaction (flash red, scale bounce, play
	// PainSound) will be wired up in the polish pass. Logged at Verbose so
	// it's silent by default but inspectable.
	UE_LOG(LogQuakeEnemy, Verbose, TEXT("%s: PlayPainReaction stub"), *GetName());
}

void AQuakeEnemyBase::PlayDeathReaction()
{
	// Stop the pawn dead where it stands. Real collapse animation (flatten
	// scale, tip over, spawn death sound) is a later polish item.
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
	}
	UE_LOG(LogQuakeEnemy, Verbose, TEXT("%s: PlayDeathReaction stub"), *GetName());
}

void AQuakeEnemyBase::Die(AController* Killer)
{

	Health = 0.f;

	// Dev-only visual indicator so a human in PIE can see kills without
	// tailing the output log. AddOnScreenDebugMessage renders straight to
	// the viewport; Key=-1 means "always add a new line, never overwrite".
	// Stripped from shipping builds alongside the DrawDebugLine calls in
	// the Axe and Grunt fire paths.
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		const FString KillerName = Killer ? Killer->GetName() : TEXT("<world>");
		GEngine->AddOnScreenDebugMessage(
			/*Key*/          -1,
			/*TimeToDisplay*/ 5.f,
			/*DisplayColor*/  FColor::Red,
			FString::Printf(TEXT("[Quake] %s killed by %s"), *GetName(), *KillerName));
	}
#endif
	UE_LOG(LogQuakeEnemy, Log, TEXT("%s killed by %s"),
		*GetName(),
		Killer ? *Killer->GetName() : TEXT("<world>"));

	// Notify the AIController to transition to Dead BEFORE we unpossess,
	// otherwise the controller still ticks on the next frame expecting a
	// live pawn. The Phase 3 functional test asserts both:
	//   (a) CurrentState == Dead
	//   (b) controller is unpossessed
	// so the ordering here is: state first, then unpossession.
	if (AQuakeEnemyAIController* QuakeAIC = Cast<AQuakeEnemyAIController>(GetController()))
	{
		QuakeAIC->NotifyPawnDied();
	}

	PlayDeathReaction();

	// Unpossess the controller once state has been recorded. The controller
	// survives the unpossession (UE keeps the AController instance around)
	// so tests can still query GetCurrentState() on it. GetPawn() on the
	// controller returns null after this call.
	if (AController* C = GetController())
	{
		C->UnPossess();
	}

	// Corpse channel flip after 2 s per SPEC section 1.6 rule 2. Until the
	// flip, the capsule still blocks rockets / hitscan / the player so a
	// fresh corpse can be cleanly hit for gib chains. After the flip, it
	// ignores everything except WorldStatic.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CorpseFlipTimerHandle,
			this,
			&AQuakeEnemyBase::OnCorpseChannelFlip,
			2.f,
			false);
	}
}

void AQuakeEnemyBase::OnCorpseChannelFlip()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule)
	{
		return;
	}

	// SPEC 1.6 rule 2: Corpse channel ignores Projectile, Weapon, Pawn, and
	// Visibility. Still blocks WorldStatic so the body doesn't fall through
	// the floor. Do the blanket-ignore first, then whitelist WorldStatic.
	Capsule->SetCollisionObjectType(QuakeCollision::ECC_Corpse);
	Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
}

float AQuakeEnemyBase::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.f || IsDead())
	{
		return 0.f;
	}

	// Shared-base CDO cast per SPEC section 1.5 — read damage-type metadata
	// uniformly without per-leaf RTTI branching.
	const UQuakeDamageType* DT = Cast<UQuakeDamageType>(
		DamageEvent.DamageTypeClass
			? DamageEvent.DamageTypeClass->GetDefaultObject()
			: UQuakeDamageType::StaticClass()->GetDefaultObject());

	// Self-damage guard: if an enemy somehow damages itself (stray rocket
	// splash, etc.), honor the damage type's bSelfDamage flag. Enemies don't
	// carry armor, so there is no absorption step — just decrement Health.
	float ScaledDamage = ActualDamage;
	if (DT)
	{
		const APawn* InstigatorPawn = EventInstigator ? EventInstigator->GetPawn() : nullptr;
		if (InstigatorPawn == this)
		{
			if (!DT->bSelfDamage)
			{
				return 0.f;
			}
			ScaledDamage *= DT->SelfDamageScale;
		}
	}

	Health = FMath::Max(0.f, Health - ScaledDamage);

	// SPEC section 3.3: "damage as a perception trigger" — notify the
	// controller unconditionally so it can promote the instigator to the
	// current target and cut straight to Chase/Attack (skipping the alert
	// pulse). Pain reaction is dispatched through the same channel so the
	// controller can suspend the FSM for 0.3 s if the roll succeeds.
	if (AQuakeEnemyAIController* QuakeAIC = Cast<AQuakeEnemyAIController>(GetController()))
	{
		QuakeAIC->OnDamaged(EventInstigator, ScaledDamage);
	}

	if (Health <= 0.f)
	{
		Die(EventInstigator);
	}

	return ScaledDamage;
}
