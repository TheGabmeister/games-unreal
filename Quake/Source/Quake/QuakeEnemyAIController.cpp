#include "QuakeEnemyAIController.h"
#include "QuakeCharacter.h"
#include "QuakeEnemyBase.h"

#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeEnemyAI, Log, All);

AQuakeEnemyAIController::AQuakeEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// --- Perception component + sense configs ---
	//
	// SPEC 3.3 wants two senses: sight (per-enemy cone and radii) and hearing
	// (walls do NOT block, so bUseLoSHearing = false). Both configs are
	// constructed here with the BASE defaults; OnPossess re-applies the per-
	// enemy radii from the pawn's UPROPERTY defaults so every enemy type
	// gets its own sight / hearing range without subclassing the controller.

	UAIPerceptionComponent* Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*Perception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2000.f;
	SightConfig->LoseSightRadius = 2200.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;  // 180° FOV cone
	SightConfig->SetMaxAge(5.f);                        // stimulus remembered 5 s
	// Detect everything — the player pawn is Neutral by default, and UE's
	// perception sense will skip it unless we opt in here.
	SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	Perception->ConfigureSense(*SightConfig);

	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 1500.f;
	// SPEC 3.3: walls do NOT block hearing — Quake-style hearing-through-walls.
	// bUseLoSHearing was deprecated in UE 5.7; the default behavior (walls do
	// NOT block hearing) is already what we want, so there's nothing to set.
	// The deprecation warning would block the "zero warnings" exit criterion.
	HearingConfig->DetectionByAffiliation.bDetectEnemies    = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals   = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	Perception->ConfigureSense(*HearingConfig);

	// Sight is the dominant sense — its stimuli "win" for target-picking
	// when sight and hearing fire in the same frame.
	Perception->SetDominantSense(UAISenseConfig_Sight::StaticClass());
}

void AQuakeEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Re-apply the per-enemy radii / angles from the pawn's UPROPERTY
	// defaults so each enemy type gets its own perception tuning without
	// needing its own controller subclass. Do this BEFORE binding the
	// delegate so the first perception sweep uses the right values.
	if (AQuakeEnemyBase* Enemy = Cast<AQuakeEnemyBase>(InPawn))
	{
		if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
		{
			if (SightConfig)
			{
				SightConfig->SightRadius = Enemy->SightRadius;
				SightConfig->LoseSightRadius = Enemy->LoseSightRadius;
				SightConfig->PeripheralVisionAngleDegrees = Enemy->PeripheralVisionAngleDegrees;
				SightConfig->SetMaxAge(Enemy->SightMaxAge);
				Perception->ConfigureSense(*SightConfig);
			}
			if (HearingConfig)
			{
				HearingConfig->HearingRange = Enemy->HearingRadius;
				Perception->ConfigureSense(*HearingConfig);
			}
		}
	}

	if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
	{
		Perception->OnTargetPerceptionUpdated.AddDynamic(
			this, &AQuakeEnemyAIController::OnTargetPerceptionUpdated);
	}

	TransitionTo(EQuakeEnemyState::Idle);
	LastAttackTime = -FLT_MAX;
}

void AQuakeEnemyAIController::OnUnPossess()
{
	if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
	{
		Perception->OnTargetPerceptionUpdated.RemoveDynamic(
			this, &AQuakeEnemyAIController::OnTargetPerceptionUpdated);
	}
	Super::OnUnPossess();
}

AQuakeEnemyBase* AQuakeEnemyAIController::GetEnemyPawn() const
{
	return Cast<AQuakeEnemyBase>(GetPawn());
}

bool AQuakeEnemyAIController::HasLineOfSightToCurrentTarget() const
{
	if (!CurrentTarget)
	{
		return false;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	const APawn* Enemy = GetPawn();
	if (!Enemy)
	{
		return false;
	}

	// Trace from the enemy's eye viewpoint to the target's current
	// location on ECC_Visibility. SPEC 1.6 matrix: Player capsule blocks
	// Visibility, world geometry blocks Visibility, corpses/projectiles
	// do not — exactly what we want for an "is the target directly
	// visible right now?" test.
	FVector EyeLoc;
	FRotator EyeRot;
	Enemy->GetActorEyesViewPoint(EyeLoc, EyeRot);
	const FVector TargetLoc = CurrentTarget->GetActorLocation();

	FCollisionQueryParams Params(SCENE_QUERY_STAT(QuakeEnemyAILoS), /*bTraceComplex*/ false);
	Params.AddIgnoredActor(Enemy);

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(
		Hit, EyeLoc, TargetLoc, ECC_Visibility, Params);
	if (!bHit)
	{
		// Nothing in between — the only way this happens is if TargetLoc
		// is inside the target's collision and the trace tunnels through,
		// which Visibility traces don't actually do. Treat as "visible"
		// for safety; the alternative (treat no-hit as no-LoS) would make
		// enemies never fire at tall-capsule pawns.
		return true;
	}
	// The first blocker must be the target itself. Anything else means a
	// wall (or another pawn) is in the way and LoS is broken.
	return Hit.GetActor() == CurrentTarget;
}

void AQuakeEnemyAIController::TransitionTo(EQuakeEnemyState NewState)
{
	if (NewState == CurrentState)
	{
		return;
	}
	UE_LOG(LogQuakeEnemyAI, Verbose, TEXT("%s: %d -> %d"),
		*GetName(), static_cast<int32>(CurrentState), static_cast<int32>(NewState));
	CurrentState = NewState;
	TimeInState = 0.f;
}

void AQuakeEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (CurrentState == EQuakeEnemyState::Dead)
	{
		return;
	}
	if (!Actor)
	{
		return;
	}

	// Phase 3 scope: only the PLAYER pawn (AQuakeCharacter) is a valid
	// target — not sibling enemies. Infighting target selection (enemy-on-
	// enemy) comes in Phase 7 and routes through the damage-driven
	// OnDamaged path anyway. The sight/hearing configs still "see"
	// everything because UE's neutral/enemy/friendly affiliation flags
	// don't affect Quake's flat no-team-id model; we filter at the
	// decision layer here instead.
	if (!Actor->IsA(AQuakeCharacter::StaticClass()))
	{
		return;
	}

	if (!Stimulus.WasSuccessfullySensed())
	{
		// Stimulus "lost" event for the current target — leave
		// LastKnownTargetLocation frozen at its last good reading so
		// Chase navigates to that spot. Clearing CurrentTarget here would
		// throw away context the FSM still needs (the next tick would
		// drop to Idle instead of moving to the last-known spot). The
		// Chase state handles "reached LKL but still no LoS → Idle".
		return;
	}

	// Successful sighting/hearing: update both the target pointer and the
	// last-known location snapshot. StimulusLocation is the perception
	// system's world-space estimate at detection time, which is what we
	// want (not the live actor position) so that losing sight freezes the
	// chase target at the moment the target went behind cover.
	CurrentTarget = Actor;
	LastKnownTargetLocation = Stimulus.StimulusLocation;
	bHasLastKnownTargetLocation = true;

	if (CurrentState == EQuakeEnemyState::Idle)
	{
		TransitionTo(EQuakeEnemyState::Alert);
	}
}

void AQuakeEnemyAIController::OnDamaged(AController* EventInstigator, float DamageAmount)
{
	if (CurrentState == EQuakeEnemyState::Dead)
	{
		return;
	}

	// Damage promotes the instigator to the current target and skips the
	// alert pulse per SPEC 3.3 "damage as a perception trigger". The
	// instigator's live location seeds LastKnownTargetLocation so Chase
	// has a place to navigate to even before a perception stimulus fires.
	APawn* InstigatorPawn = EventInstigator ? EventInstigator->GetPawn() : nullptr;
	if (InstigatorPawn)
	{
		CurrentTarget = InstigatorPawn;
		LastKnownTargetLocation = InstigatorPawn->GetActorLocation();
		bHasLastKnownTargetLocation = true;
	}

	// Pain roll: SPEC 3.3 pain_chance = min(0.8, damage / max_health * 2).
	// The formula lives on the pawn (pure static) so unit tests can exercise
	// it directly — we just consume the result here.
	if (AQuakeEnemyBase* Enemy = GetEnemyPawn())
	{
		const float PainChance = AQuakeEnemyBase::ComputePainChance(DamageAmount, Enemy->GetMaxHealth());
		if (PainChance > 0.f && FMath::FRand() < PainChance)
		{
			Enemy->PlayPainReaction();
			TransitionTo(EQuakeEnemyState::Pain);
			return;
		}
	}

	// No pain flinch: cut straight to Chase (range check happens next tick).
	if (CurrentState == EQuakeEnemyState::Idle || CurrentState == EQuakeEnemyState::Alert)
	{
		TransitionTo(EQuakeEnemyState::Chase);
	}
}

void AQuakeEnemyAIController::NotifyPawnDied()
{
	TransitionTo(EQuakeEnemyState::Dead);
	// Stop any active path-follow request so the CMC goes quiet. The pawn
	// unpossesses us as the next step of the death flow (see
	// AQuakeEnemyBase::Die).
	StopMovement();
}

void AQuakeEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EQuakeEnemyState::Dead)
	{
		return;
	}

	TimeInState += DeltaTime;

	AQuakeEnemyBase* Enemy = GetEnemyPawn();
	if (!Enemy || Enemy->IsDead())
	{
		return;
	}

	switch (CurrentState)
	{
	case EQuakeEnemyState::Idle:
		// Passive — wait for perception stimulus or OnDamaged to promote us.
		break;

	case EQuakeEnemyState::Alert:
		if (TimeInState >= AlertPulseDuration)
		{
			TransitionTo(EQuakeEnemyState::Chase);
		}
		break;

	case EQuakeEnemyState::Chase:
	{
		// Dropped target or never had a last-known location → back to Idle.
		if (!CurrentTarget || !bHasLastKnownTargetLocation)
		{
			StopMovement();
			CurrentTarget = nullptr;
			bHasLastKnownTargetLocation = false;
			TransitionTo(EQuakeEnemyState::Idle);
			break;
		}

		const bool bHasLoS = HasLineOfSightToCurrentTarget();
		const FVector EnemyLoc = Enemy->GetActorLocation();

		// "In range + LoS" is SPEC 3.3 Attack contract — only promote to
		// Attack when BOTH hold. Distance is computed against the live
		// target actor (we're re-acquiring right now, so the stale LKL
		// is irrelevant for the range check).
		if (bHasLoS)
		{
			const float DistToTarget = FVector::Dist(EnemyLoc, CurrentTarget->GetActorLocation());
			if (DistToTarget <= Enemy->AttackRange)
			{
				// Cancel the active Chase MoveTo before firing so the
				// pawn doesn't slide while "attacking". Attack exits
				// back to Chase if range or LoS is lost, which will
				// reissue MoveTo there.
				StopMovement();
				TransitionTo(EQuakeEnemyState::Attack);
				break;
			}
		}

		// No LoS (or LoS + out of range): navigate to the last known
		// location. This is the fix for "grunts tracking the player
		// through walls" — we read the frozen LKL, NOT the live actor
		// position. Chase refreshes LKL on every successful sight
		// stimulus (OnTargetPerceptionUpdated), so a visible moving
		// target stays tracked smoothly.
		const float DistToLKL = FVector::Dist(EnemyLoc, LastKnownTargetLocation);

		// Reached the last-known spot without re-acquiring — drop the
		// target. Without this branch the enemy would get stuck moving
		// to a point that is already exactly under its feet. 100 u is
		// roughly 2 player-capsule widths and is comfortably larger
		// than nav-mesh arrival tolerance.
		if (!bHasLoS && DistToLKL < 100.f)
		{
			StopMovement();
			CurrentTarget = nullptr;
			bHasLastKnownTargetLocation = false;
			TransitionTo(EQuakeEnemyState::Idle);
			break;
		}

		Enemy->MoveToTarget(LastKnownTargetLocation);
		break;
	}

	case EQuakeEnemyState::Attack:
	{
		if (!CurrentTarget)
		{
			TransitionTo(EQuakeEnemyState::Idle);
			break;
		}

		// SPEC 3.3 Attack contract: range + LoS. If EITHER check fails,
		// fall back to Chase so the controller repositions to LKL (or
		// re-acquires) before firing again — this is what prevents an
		// enemy from sitting in Attack firing into a wall after a
		// target-behind-cover transition.
		const float DistToTarget = FVector::Dist(
			Enemy->GetActorLocation(),
			CurrentTarget->GetActorLocation());
		const bool bHasLoS = HasLineOfSightToCurrentTarget();
		if (DistToTarget > Enemy->AttackRange || !bHasLoS)
		{
			TransitionTo(EQuakeEnemyState::Chase);
			break;
		}

		// Gated fire.
		const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
		if ((Now - LastAttackTime) >= Enemy->AttackCooldown)
		{
			Enemy->FireAtTarget(CurrentTarget);
			LastAttackTime = Now;
		}
		break;
	}

	case EQuakeEnemyState::Pain:
		if (TimeInState >= PainDuration)
		{
			TransitionTo(CurrentTarget ? EQuakeEnemyState::Chase : EQuakeEnemyState::Idle);
		}
		break;

	default:
		break;
	}
}
