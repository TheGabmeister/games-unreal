#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "QuakeEnemyAIController.generated.h"

class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class AQuakeEnemyBase;

/**
 * Enemy finite-state machine states. SPEC section 3.3:
 *     Idle → Alert → Chase → Attack → (Pain) → Dead
 *
 * - Idle   : stationary / patrolling. Promoted to Alert on first valid stim.
 * - Alert  : 0.5 s pulse (scale animation + sound stub), then → Chase.
 * - Chase  : MoveTo the target. Promotes to Attack when in range + LoS.
 * - Attack : fire the pawn's attack on cooldown. Falls back to Chase when
 *            the target leaves attack range.
 * - Pain   : brief 0.3 s FSM suspension after being hit (roll-gated via
 *            AQuakeEnemyBase::ComputePainChance). Bosses skip this state.
 * - Dead   : terminal. Controller stops ticking; pawn is unpossessed.
 */
UENUM(BlueprintType)
enum class EQuakeEnemyState : uint8
{
	Idle,
	Alert,
	Chase,
	Attack,
	Pain,
	Dead
};

/**
 * Phase 3 brain class (the controller half of the body/brain split).
 * Subclassed per enemy type (AQuakeAIController_Grunt, _Knight, _Ogre, ...)
 * when behavior diverges from the baseline FSM.
 *
 * Architecture (SPEC section 3.3 + [CLAUDE.md] "Architecture: AI Split"):
 *
 *   - This class owns the perception component, target tracking, and the
 *     state machine. It does NOT read health or position directly from the
 *     pawn for decisions beyond "am I in range" — all actions route through
 *     the pawn's action methods (MoveToTarget, FireAtTarget, PlayPainReaction,
 *     PlayDeathReaction).
 *   - Per-type behavior variations (Fiend leap, Ogre grenade arc, Zombie
 *     revive) live on AIController subclasses, never on pawn subclasses.
 *
 * The FSM is Tick-driven — no Behavior Trees. This is the simplest thing
 * that works for Quake's fixed per-type patterns and avoids the BT asset
 * round-trip overhead for trivial transitions.
 */
UCLASS()
class QUAKE_API AQuakeEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AQuakeEnemyAIController();

	/** Sight sense config (owned, configured in the ctor, re-tuned on possess). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	/** Hearing sense config (walls do NOT block — bUseLoSHearing=false). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	// --- State accessors ---

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	EQuakeEnemyState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	AActor* GetCurrentTarget() const { return CurrentTarget; }

	// --- Entry points from the pawn ---

	/**
	 * Called by AQuakeEnemyBase::TakeDamage when the pawn is hurt. Promotes
	 * the instigator's pawn to the current target, skips the alert pulse,
	 * and (conditionally, via the pain chance roll from SPEC 3.3) drops the
	 * FSM into the Pain state for 0.3 s.
	 */
	virtual void OnDamaged(AController* EventInstigator, float DamageAmount);

	/**
	 * Called by AQuakeEnemyBase::Die. Transitions to Dead and stops the
	 * FSM; the pawn then unpossesses us as the second step of the death
	 * flow. Idempotent — safe to call multiple times.
	 */
	virtual void NotifyPawnDied();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	/** State-transition helper: resets TimeInState. */
	void TransitionTo(EQuakeEnemyState NewState);

	/** Helper: cast GetPawn() to AQuakeEnemyBase once. */
	AQuakeEnemyBase* GetEnemyPawn() const;

	/**
	 * Line-of-sight test from the enemy's eye viewpoint to CurrentTarget's
	 * location, traced on `ECC_Visibility`. Returns true iff the trace
	 * reaches the target without being blocked by world geometry. The
	 * enemy's own capsule is added to the ignore list.
	 *
	 * SPEC 3.3 Attack state requires "in range + LoS" — this is the LoS
	 * side of that gate. Also used in Chase to avoid promoting to Attack
	 * when the target ducks behind a wall.
	 */
	bool HasLineOfSightToCurrentTarget() const;

	// --- Tuning (defaults can be overridden per enemy controller subclass) ---

	/** Seconds the Alert pulse lasts before transitioning to Chase. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|State", meta = (ClampMin = "0.0"))
	float AlertPulseDuration = 0.5f;

	/** Seconds the Pain state suspends the FSM. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|State", meta = (ClampMin = "0.0"))
	float PainDuration = 0.3f;

	// --- Runtime state ---

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|State")
	EQuakeEnemyState CurrentState = EQuakeEnemyState::Idle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|State")
	TObjectPtr<AActor> CurrentTarget = nullptr;

	/**
	 * World-space position of the most recent successful perception or
	 * damage event involving CurrentTarget. Chase navigates to THIS point
	 * rather than the live target actor — when the target ducks behind a
	 * wall, the enemy moves toward where the target WAS and drops back to
	 * Idle if it reaches that spot without re-acquiring. Without this,
	 * once a grunt saw the player once it would track the player's live
	 * position through walls forever.
	 */
	FVector LastKnownTargetLocation = FVector::ZeroVector;

	/** Whether LastKnownTargetLocation holds a meaningful value. */
	bool bHasLastKnownTargetLocation = false;

	/** Seconds spent in the current state — used by Alert / Pain timeouts. */
	float TimeInState = 0.f;

	/** World time of the last successful attack fire, -inf initially. */
	float LastAttackTime = -FLT_MAX;

	// --- Infighting (Phase 7, SPEC 3.3) ---

	/**
	 * SPEC 3.3: 10-second grudge timer. When an enemy damages us, we switch
	 * target to that enemy. After GrudgeDuration seconds (or if the grudge
	 * target dies / we lose sight for 5 s), we revert to the player.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Infighting", meta = (ClampMin = "0.0"))
	float GrudgeDuration = 10.f;

	/** The actor we're grudge-targeting (another enemy). Null = no grudge. */
	TWeakObjectPtr<AActor> GrudgeTarget;

	/** World time when the current grudge expires. */
	double GrudgeExpireTime = 0.0;
};
