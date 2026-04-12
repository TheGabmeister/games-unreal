#include "QuakeEnemyBase.h"

#include "QuakeBalanceRows.h"
#include "QuakeCollisionChannels.h"
#include "QuakeDamageType.h"
#include "QuakeEnemyAIController.h"
#include "QuakeProjectSettings.h"

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

void AQuakeEnemyBase::PostInitializeComponents()
{
	// Load balance data BEFORE Super. Super::PostInitializeComponents calls
	// SpawnDefaultController -> OnPossess, which reads our perception stats
	// (SightRadius, HearingRadius, etc.) to configure the AI sense configs.
	// The stats must already reflect DataTable values at that point.
	LoadStatsFromDataTable();

	Super::PostInitializeComponents();
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

void AQuakeEnemyBase::LoadStatsFromDataTable()
{
	if (StatsRowName.IsNone())
	{
		return;
	}

	const UQuakeProjectSettings* Settings = GetDefault<UQuakeProjectSettings>();
	UDataTable* Table = Settings->EnemyStatsTable.LoadSynchronous();
	if (!Table)
	{
		return;
	}

	const FQuakeEnemyStatsRow* Row = Table->FindRow<FQuakeEnemyStatsRow>(
		StatsRowName, TEXT("AQuakeEnemyBase::LoadStatsFromDataTable"));
	if (!Row)
	{
		UE_LOG(LogQuakeEnemy, Warning,
			TEXT("%s: StatsRowName '%s' not found in EnemyStatsTable — using C++ defaults."),
			*GetName(), *StatsRowName.ToString());
		return;
	}

	MaxHealth                    = Row->MaxHealth;
	WalkSpeed                    = Row->WalkSpeed;
	SightRadius                  = Row->SightRadius;
	LoseSightRadius              = Row->LoseSightRadius;
	PeripheralVisionAngleDegrees = Row->PeripheralVisionAngleDegrees;
	HearingRadius                = Row->HearingRadius;
	SightMaxAge                  = Row->SightMaxAge;
	AttackRange                  = Row->AttackRange;
	AttackDamage                 = Row->AttackDamage;
	AttackCooldown               = Row->AttackCooldown;
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

void AQuakeEnemyBase::Die(AController* Killer, bool bGibbed)
{
	if (IsDead()) return;

	Health = 0.f;

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		const FString KillerName = Killer ? Killer->GetName() : TEXT("<world>");
		GEngine->AddOnScreenDebugMessage(
			/*Key*/          -1,
			/*TimeToDisplay*/ 5.f,
			/*DisplayColor*/  FColor::Red,
			FString::Printf(TEXT("[Quake] %s %s by %s"),
				*GetName(), bGibbed ? TEXT("GIBBED") : TEXT("killed"), *KillerName));
	}
#endif
	UE_LOG(LogQuakeEnemy, Log, TEXT("%s %s by %s"),
		*GetName(),
		bGibbed ? TEXT("gibbed") : TEXT("killed"),
		Killer ? *Killer->GetName() : TEXT("<world>"));

	// SPEC 3.4: gibbed enemies do not drop loot.
	if (!bGibbed)
	{
		SpawnDrops();
	}

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

	if (bGibbed)
	{
		PlayGibReaction();
	}
	else
	{
		PlayDeathReaction();
	}

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

void AQuakeEnemyBase::PlayGibReaction()
{
	// Phase 7 stub — real gib scattering (fling mesh pieces, blood decal,
	// fade-out) is a later polish item. For now, immediately hide the actor
	// (the corpse timer still runs so cleanup is consistent).
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
	}
	SetActorHiddenInGame(true);
	UE_LOG(LogQuakeEnemy, Verbose, TEXT("%s: PlayGibReaction stub (hidden)"), *GetName());
}

void AQuakeEnemyBase::SpawnDrops()
{
	UWorld* World = GetWorld();
	if (!World || DropTable.Num() == 0)
	{
		return;
	}

	const FVector SpawnLoc = GetActorLocation();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (const FQuakeDropEntry& Entry : DropTable)
	{
		if (!Entry.PickupClass)
		{
			continue;
		}
		// Roll the chance. 1.0 always succeeds; 0.0 never.
		if (Entry.Chance < 1.f && FMath::FRand() >= Entry.Chance)
		{
			continue;
		}
		AActor* Pickup = World->SpawnActor<AActor>(Entry.PickupClass, SpawnLoc, FRotator::ZeroRotator, Params);
		if (Pickup)
		{
			UE_LOG(LogQuakeEnemy, Verbose, TEXT("%s: dropped %s"), *GetName(), *GetNameSafe(Pickup));
		}
	}
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
		// SPEC 3.4: gib when overkill damage >= 2× remaining HP before this
		// hit. "Remaining HP before" = Health + ScaledDamage (Health is already
		// decremented above). HealthBefore <= 0 can't happen — IsDead() early-
		// returns at the top.
		const float HealthBefore = Health + ScaledDamage;
		const float Overkill = ScaledDamage - HealthBefore;  // excess past 0
		const bool bGibbed = Overkill >= HealthBefore * 2.f;
		Die(EventInstigator, bGibbed);
	}

	return ScaledDamage;
}
