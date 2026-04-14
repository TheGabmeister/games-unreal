#include "QuakeEnemyBase.h"

#include "QuakeBalanceRows.h"
#include "QuakeCollisionChannels.h"
#include "QuakeDamageType.h"
#include "QuakeEnemyAIController.h"
#include "QuakeGameMode.h"
#include "QuakePlayerState.h"
#include "QuakeProjectSettings.h"
#include "QuakeSoundManager.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
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

	// DESIGN 6.1: difficulty scaling bakes into MaxHealth before we seed
	// Health from it. Pulls multipliers from the authoritative GameMode.
	if (const UWorld* World = GetWorld())
	{
		if (const AQuakeGameMode* GM = World->GetAuthGameMode<AQuakeGameMode>())
		{
			ApplyDifficultyScaling(GM->GetDifficultyMultipliers());
		}
	}

	SetHealth(MaxHealth);

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

void AQuakeEnemyBase::ComputeScaledEnemyStats(
	float BaseMaxHealth,
	const FQuakeDifficultyMultipliers& Multipliers,
	float& OutMaxHealth,
	float& OutAttackDamageMultiplier)
{
	OutMaxHealth              = BaseMaxHealth * Multipliers.EnemyHP;
	OutAttackDamageMultiplier = Multipliers.EnemyDamage;
}

void AQuakeEnemyBase::ApplyDifficultyScaling(const FQuakeDifficultyMultipliers& Multipliers)
{
	float ScaledMaxHealth = MaxHealth;
	ComputeScaledEnemyStats(MaxHealth, Multipliers, ScaledMaxHealth, AttackDamageMultiplier);
	MaxHealth = ScaledMaxHealth;
}

void AQuakeEnemyBase::SetHealth(float NewValue)
{
	Health = FMath::Max(0.f, NewValue);
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
	// Visual flinch only — the pain SOUND is emitted from TakeDamage's
	// non-fatal branch so it fires once per hit. PlayPainReaction is gated
	// by the SPEC 3.3 pain_chance roll AND can fire on the killing hit
	// (the FSM transitions to Pain before Die overrides it), so emitting
	// sound here would double up on non-fatal hits and play pain on top of
	// the death sound on fatal hits.
	UE_LOG(LogQuakeEnemy, Verbose, TEXT("%s: PlayPainReaction"), *GetName());
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
	SetHealth(0.f);

	// SPEC 5.9 player-credit rules. Level-clear is independent of credit
	// (it reads spawn-point satisfaction), so we only touch PlayerState
	// when this enemy actually counts AND the player deserves the point:
	//   - Normal kill: Killer is the player's controller.
	//   - Hazard kill: Killer is null (world damage) AND the player
	//     previously damaged this enemy during the level attempt.
	// The infighting-chain 5-second rule is deferred — infighting kills
	// currently credit nothing, which matches the conservative branch of
	// the SPEC table ("dead but uncredited").
	if (bIsMarkedKillTarget)
	{
		bool bCreditPlayer = false;
		if (Killer && Killer->IsPlayerController())
		{
			bCreditPlayer = true;
		}
		else if (!Killer && bPlayerHasDamagedMe)
		{
			bCreditPlayer = true;
		}

		if (bCreditPlayer)
		{
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
			{
				if (AQuakePlayerState* PS = PC->GetPlayerState<AQuakePlayerState>())
				{
					PS->AddKillCredit();
				}
			}
		}
	}

	// SPEC section 3.5: death sound fires once for both collapse and gib
	// deaths. Phase 14: routed through the sound manager (DT_SoundEvents row).
	UQuakeSoundManager::PlaySoundEvent(this, EQuakeSoundEvent::EnemyDeath, GetActorLocation());

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

	SetHealth(Health - ScaledDamage);

	// SPEC 5.9 hazard-credit rule: remember if the player ever damaged this
	// enemy. A later lava / slime kill credits the player iff this flag is
	// set at the moment of death.
	if (EventInstigator && EventInstigator->IsPlayerController())
	{
		bPlayerHasDamagedMe = true;
	}

	// SPEC 3.5 / Phase 14: pain sound on every non-fatal hit, independent of
	// the pain-chance flinch roll. EnemyDeath fires from Die() so the fatal
	// hit doesn't double-play pain + death. The pain reaction itself
	// (PlayPainReaction) ALSO emits EnemyPain when the flinch roll succeeds —
	// duplicate audio is acceptable for v1; later phases can dedupe on the
	// per-enemy cooldown side.
	if (Health > 0.f)
	{
		UQuakeSoundManager::PlaySoundEvent(this, EQuakeSoundEvent::EnemyPain, GetActorLocation());
	}

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
