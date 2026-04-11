#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "QuakeEnemyBase.generated.h"

class UStaticMeshComponent;
class USoundBase;

/**
 * Phase 3 body class (the pawn half of the body/brain split) for every
 * Quake enemy. Subclassed per enemy type (AQuakeEnemy_Grunt, _Knight, _Ogre,
 * _Fiend, _Shambler, _Zombie) — each leaf sets the SPEC section 3.1 stats
 * as UPROPERTY defaults in its constructor and assigns
 * AIControllerClass = AQuakeAIController_<Type>::StaticClass().
 *
 * Architecture (see SPEC section 3.3 and [CLAUDE.md] "Architecture: AI Split"):
 *
 *   - The pawn ("body") owns the capsule, primitive mesh components, health,
 *     and the attack-action hooks that the controller invokes. It does NOT
 *     make decisions — there is no Tick-driven target evaluation here.
 *   - The AIController ("brain") owns the state machine, perception, and
 *     decision logic. It calls the pawn's action methods to drive behavior.
 *
 * The split lets UE's AI debugger (' key in PIE) show state on the controller
 * and enables swapping controllers (e.g., a "spectate" controller) without
 * rewriting the body. It also mirrors standard UE conventions.
 *
 * Blueprint subclasses (BP_Enemy_*) only fill in asset slots — meshes,
 * materials, pain/death sound slots, drop table entries (Phase 7). Zero
 * nodes in the BP event graph per the project convention.
 */
UCLASS(Abstract)
class QUAKE_API AQuakeEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AQuakeEnemyBase();

	// --- Mesh component slots (primitive shapes per SPEC section 3.0) ---

	/** Body capsule visual. Attached to the character capsule root. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Mesh")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	/** Head sphere. Attached to BodyMesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Mesh")
	TObjectPtr<UStaticMeshComponent> HeadMesh;

	/** Weapon box (Grunt rifle, Ogre chainsaw, etc.). Attached to BodyMesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Mesh")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	// --- Sound slots, filled in the BP subclass ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Sound")
	TObjectPtr<USoundBase> PainSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Sound")
	TObjectPtr<USoundBase> DeathSound;

	// --- Perception tuning, read by the AIController on possess ---

	/** Sight radius from SPEC section 3.1 (unreal units). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Perception", meta = (ClampMin = "0.0"))
	float SightRadius = 2000.f;

	/** Distance past SightRadius before the target is considered lost. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Perception", meta = (ClampMin = "0.0"))
	float LoseSightRadius = 2200.f;

	/**
	 * Peripheral vision half-angle in degrees from the forward vector.
	 * Matches UE's UAISenseConfig_Sight::PeripheralVisionAngleDegrees directly.
	 * SPEC: 90° → 180° total FOV cone.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Perception", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float PeripheralVisionAngleDegrees = 90.f;

	/**
	 * Hearing radius from SPEC section 3.1. Walls do NOT block hearing
	 * (SPEC: bUseLoSHearing = false on the sense config). The AIController
	 * applies this to its UAISenseConfig_Hearing on possess.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Perception", meta = (ClampMin = "0.0"))
	float HearingRadius = 1500.f;

	/** Seconds a stimulus is remembered after fading out of sight. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Perception", meta = (ClampMin = "0.0"))
	float SightMaxAge = 5.f;

	// --- Attack tuning ---

	/** Maximum attack range in unreal units. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0"))
	float AttackRange = 1500.f;

	/** Damage per successful attack. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.0"))
	float AttackDamage = 4.f;

	/** Seconds between attacks (1 / rate-of-fire). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Attack", meta = (ClampMin = "0.01"))
	float AttackCooldown = 1.5f;

	// --- Movement tuning ---

	/** Ground walk speed. Wired to UCharacterMovementComponent::MaxWalkSpeed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Movement", meta = (ClampMin = "0.0"))
	float WalkSpeed = 300.f;

	// --- Health ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 30.f;

	UFUNCTION(BlueprintCallable, Category = "Enemy|Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category = "Enemy|Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Enemy|Health")
	bool IsDead() const { return Health <= 0.f; }

	/**
	 * Pain chance formula from SPEC section 3.3:
	 *     chance = min(0.8, (damage / max_health) * 2)
	 *
	 * Extracted as a pure static function so unit tests can exercise the
	 * formula without spinning up a world, a pawn, or a controller — the
	 * same pattern used by AQuakeCharacter::ApplyArmorAbsorption and
	 * UQuakeCharacterMovementComponent::ApplyQuakeAirAccel.
	 *
	 * MaxHealth <= 0 degenerately returns 0 rather than dividing by zero.
	 */
	static float ComputePainChance(float Damage, float InMaxHealth);

	// --- Action methods called by the AIController ---

	/** Issue a navmesh MoveTo request via the AAIController. */
	virtual void MoveToTarget(const FVector& TargetLocation);

	/**
	 * Execute the per-type attack (hitscan for Grunt, melee for Knight,
	 * projectile for Ogre, etc.). PURE_VIRTUAL (not C++ `= 0`) because
	 * UCLASS reflection constructs a CDO even for Abstract classes — the
	 * engine macro emits a crashing stub body that satisfies the CDO
	 * constructor while still firing at runtime if anything reaches it.
	 * Same gotcha as AQuakeWeaponBase::Fire.
	 */
	virtual void FireAtTarget(AActor* Target) PURE_VIRTUAL(AQuakeEnemyBase::FireAtTarget, );

	/** Pain reaction stub (flash, scale, sound). Overridable by subclasses. */
	virtual void PlayPainReaction();

	/** Death reaction stub (collapse, sound). Overridable by subclasses. */
	virtual void PlayDeathReaction();

	// --- Death ---

	/**
	 * Kill this enemy immediately. Called from TakeDamage when Health <= 0.
	 * Notifies the controller, stops movement, plays death reaction, and
	 * schedules the 2-second corpse channel flip per SPEC section 1.6 rule 2.
	 * Also unpossesses the AIController as part of the Dead state transition.
	 */
	virtual void Die(AController* Killer);

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	/** Live HP. Mutated ONLY by TakeDamage / Die, per the SPEC rule. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy|Health")
	float Health = 30.f;

	/** Timer callback 2 s after death: flip capsule to the Corpse channel. */
	UFUNCTION()
	void OnCorpseChannelFlip();

private:
	FTimerHandle CorpseFlipTimerHandle;
};
