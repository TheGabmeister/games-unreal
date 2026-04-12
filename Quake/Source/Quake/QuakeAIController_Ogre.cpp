#include "QuakeAIController_Ogre.h"

#include "QuakeEnemy_Ogre.h"

#include "Engine/World.h"

AQuakeAIController_Ogre::AQuakeAIController_Ogre()
{
	// Uses base perception + FSM. Only the Attack state is overridden.
}

void AQuakeAIController_Ogre::Tick(float DeltaTime)
{
	// For non-Attack states, the base FSM handles everything.
	if (CurrentState != EQuakeEnemyState::Attack)
	{
		Super::Tick(DeltaTime);
		return;
	}

	// --- Attack state override ---
	// We must NOT call Super::Tick here because the base Attack case would
	// also call FireAtTarget, causing a double-fire. Instead, replicate the
	// bookkeeping that the base Tick does before the state switch.
	AAIController::Tick(DeltaTime);

	if (CurrentState == EQuakeEnemyState::Dead)
	{
		return;
	}

	TimeInState += DeltaTime;

	AQuakeEnemy_Ogre* Ogre = Cast<AQuakeEnemy_Ogre>(GetPawn());
	if (!Ogre || Ogre->IsDead() || !CurrentTarget)
	{
		TransitionTo(EQuakeEnemyState::Idle);
		return;
	}

	// Range + LoS check, same as the base Attack state.
	const float DistToTarget = FVector::Dist(
		Ogre->GetActorLocation(), CurrentTarget->GetActorLocation());
	const bool bHasLoS = HasLineOfSightToCurrentTarget();
	if (DistToTarget > Ogre->AttackRange || !bHasLoS)
	{
		TransitionTo(EQuakeEnemyState::Chase);
		return;
	}

	// Gated fire — choose melee vs grenade based on distance.
	const UWorld* World = GetWorld();
	if (!World) return;

	const float Now = World->GetTimeSeconds();
	if ((Now - LastAttackTime) < Ogre->AttackCooldown)
	{
		return;
	}

	if (DistToTarget <= MeleeThreshold)
	{
		Ogre->FireAtTarget(CurrentTarget);
	}
	else
	{
		Ogre->FireGrenadeAtTarget(CurrentTarget);
	}
	LastAttackTime = Now;
}
