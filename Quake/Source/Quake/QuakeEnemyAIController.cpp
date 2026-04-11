#include "QuakeEnemyAIController.h"
#include "QuakeEnemyBase.h"

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
	if (!Actor || !Stimulus.WasSuccessfullySensed())
	{
		// Stimulus "lost" event — leave current target alone for now. The
		// 5 s stimulus aging (SightMaxAge) already handles "forget after
		// losing sight" at the perception layer.
		return;
	}

	// Phase 3 scope: only the player pawn is a valid target. Infighting
	// target selection (enemy-on-enemy) comes in Phase 7 and will route
	// through the damage-driven OnDamaged path anyway.
	if (!Actor->IsA(APawn::StaticClass()))
	{
		return;
	}

	CurrentTarget = Actor;

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
	// alert pulse per SPEC 3.3 "damage as a perception trigger".
	APawn* InstigatorPawn = EventInstigator ? EventInstigator->GetPawn() : nullptr;
	if (InstigatorPawn)
	{
		CurrentTarget = InstigatorPawn;
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
		if (!CurrentTarget)
		{
			TransitionTo(EQuakeEnemyState::Idle);
			break;
		}
		const float DistToTarget = FVector::Dist(
			Enemy->GetActorLocation(),
			CurrentTarget->GetActorLocation());
		if (DistToTarget <= Enemy->AttackRange)
		{
			// Simple line-of-sight check — the full enemy-to-target trace is
			// a Phase 7 refinement. For Phase 3, in-range = attack.
			TransitionTo(EQuakeEnemyState::Attack);
		}
		else
		{
			Enemy->MoveToTarget(CurrentTarget->GetActorLocation());
		}
		break;
	}

	case EQuakeEnemyState::Attack:
	{
		if (!CurrentTarget)
		{
			TransitionTo(EQuakeEnemyState::Idle);
			break;
		}
		const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
		if ((Now - LastAttackTime) >= Enemy->AttackCooldown)
		{
			Enemy->FireAtTarget(CurrentTarget);
			LastAttackTime = Now;
		}
		const float DistToTarget = FVector::Dist(
			Enemy->GetActorLocation(),
			CurrentTarget->GetActorLocation());
		if (DistToTarget > Enemy->AttackRange)
		{
			TransitionTo(EQuakeEnemyState::Chase);
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
