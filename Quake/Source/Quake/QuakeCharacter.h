#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "QuakeAmmoType.h"
#include "QuakeKeyColor.h"
#include "QuakeCharacter.generated.h"

class UCameraComponent;
class UPostProcessComponent;
class AQuakeWeaponBase;

UCLASS()
class QUAKE_API AQuakeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AQuakeCharacter(const FObjectInitializer& ObjectInitializer);

	/** SPEC 2.0: 8 weapon slots indexed by their number key (1..8). */
	static constexpr int32 NumWeaponSlots = 8;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UPostProcessComponent> DamageFlashPostProcess;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float LookSensitivity = 0.5f;

	// --- Phase 2: health + damage pipeline ---

	/** Read-only accessor for the HUD overlay. SPEC: only TakeDamage mutates Health. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead() const { return Health <= 0.f; }

	/**
	 * Overcharge ceiling for Megahealth / stimpack-style pickups. SPEC
	 * section 1.2: HP can go up to 200 via Megahealth and decays back to
	 * 100 at 1 HP/sec. Pickup CanBeConsumedBy uses this to reject a
	 * Megahealth touch when the player is already at 200.
	 */
	static constexpr float GetOverchargeCap() { return 200.f; }

	// --- Phase 4: weapon slot array ---

	/**
	 * Per-slot spawned weapon actors. Index maps to SPEC 2.0 weapon number
	 * (0 = slot 1 Axe, 1 = slot 2 Shotgun, 3 = slot 4 Nailgun, etc.).
	 * Parallel to UQuakeGameInstance::OwnedWeaponClasses. Null entries
	 * mean "not owned". Size is fixed to 8 in BeginPlay.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapons")
	TArray<TObjectPtr<AQuakeWeaponBase>> WeaponInstances;

	/** Zero-based slot index of the currently-equipped weapon. -1 = none. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapons")
	int32 CurrentWeaponSlot = -1;

	/** The weapon the HUD / fire path uses. Points into WeaponInstances or null. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapons")
	TObjectPtr<AQuakeWeaponBase> CurrentWeapon;

	/**
	 * Hide the old weapon, show the slot's weapon, update CurrentWeapon.
	 * No-op on out-of-range or unowned slots. Instant swap — SPEC 2.2's
	 * 0.2s lower + 0.2s raise animation is polish for a later phase.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool SwitchToWeaponSlot(int32 SlotIndexZeroBased);

	// --- Phase 4: inventory facade ---

	/** Grant ammo via the GameInstance. Returns the amount actually added (capped). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GiveAmmo(EQuakeAmmoType Type, int32 Amount);

	/** Consume ammo via the GameInstance. Returns true if the full amount was available. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool ConsumeAmmo(EQuakeAmmoType Type, int32 Amount);

	/** Read ammo count via the GameInstance — convenience for the HUD and pickups. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetAmmo(EQuakeAmmoType Type) const;

	/**
	 * Grant health directly to the Character. Self-owned state, so this
	 * does NOT route through the GameInstance (health isn't persistent
	 * inventory). bOvercharge allows the grant to push HP past MaxHealth
	 * up to GetOverchargeCap().
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void GiveHealth(float Amount, bool bOvercharge);

	/**
	 * SPEC 4.3: Quad Damage multiplies outgoing weapon damage by 4 for 30 s.
	 * Weapons query this right before calling ApplyPointDamage /
	 * ApplyRadialDamageWithFalloff — projectile weapons (RL, Nailgun) also
	 * bake the scale into the spawned projectile's DamageScale at launch
	 * time so the multiplier is frozen on the shot, not re-sampled at impact
	 * (matches Quake: a rocket fired during Quad lands at 4× even if the
	 * timer ticks out mid-flight). Returns 1.0 when no Quad entry is active
	 * or when no PlayerState exists yet.
	 *
	 * "Does not affect splash self-damage scale" per SPEC 4.3 — the
	 * projectile's DamageScale scales BaseDamage; self-damage multiplies
	 * that further by UQuakeDamageType::SelfDamageScale inside TakeDamage,
	 * so the behavior comes out of the existing pipeline for free.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float GetOutgoingDamageScale() const;

	/** Facade to PlayerState->GiveKey; called from AQuakePickup_Key. */
	UFUNCTION(BlueprintCallable, Category = "Keys")
	void GiveKey(EQuakeKeyColor Color);

	/** Facade to PlayerState->HasKey — lets callers route through the pawn. */
	UFUNCTION(BlueprintCallable, Category = "Keys")
	bool HasKey(EQuakeKeyColor Color) const;

	/**
	 * Pickup-driven weapon grant. SPEC 2.2 "first pickup grants weapon +
	 * ammo and auto-switches; subsequent pickup grants only ammo". Returns
	 * true iff this was the first time the slot was filled (i.e. the
	 * weapon actor was spawned and the player auto-switched).
	 *
	 * Writes the class to the GameInstance so death-respawn re-seeds from
	 * the persisted inventory rather than the starting loadout.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool GiveWeaponPickup(int32 SlotIndexZeroBased, TSubclassOf<class AQuakeWeaponBase> WeaponClass);

	/**
	 * Auto-switch to the best owned weapon that has ammo when the current
	 * weapon fires on empty. Walks the SPEC 2.2 priority list
	 * (RL → SNG → SSG → NG → SG → Axe, explicitly NOT Thunderbolt or GL
	 * which are "kept manual" per SPEC), skipping the current slot and any
	 * slot that is unowned or has insufficient ammo. Returns true iff a
	 * switch happened.
	 *
	 * Called from AQuakeWeaponBase::TryFire when ConsumeAmmo returns false,
	 * right after PlayEmptyClick. The click still plays so the player gets
	 * audible feedback for the failed shot before the swap.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool AutoSwitchFromEmptyWeapon();

	/**
	 * Pure static helper behind AutoSwitchFromEmptyWeapon. Walks SPEC 2.2
	 * priority [RL, SNG, SSG, NG, SG, Axe] and returns the first slot
	 * index that is both owned and has fireable ammo, excluding
	 * ExcludeSlot. Returns -1 if nothing is eligible.
	 *
	 * The inputs are parallel length-8 bool arrays (index i corresponds to
	 * SPEC 2.0 weapon slot i+1). Extracted as a pure helper so tests can
	 * exercise the priority table without spinning up a world, a character,
	 * or a GameInstance — same pattern as ApplyArmorAbsorption,
	 * ApplyQuakeAirAccel, ComputePainChance, and ComputeLinearFalloffDamage.
	 */
	static int32 PickAutoSwitchWeaponSlot(
		const TArray<bool>& SlotOwnedMask,
		const TArray<bool>& SlotHasAmmoMask,
		int32 ExcludeSlot);

	/**
	 * Damage absorption math, extracted as a pure static helper so it can
	 * be unit-tested without spinning up a world or a character. Implements
	 * the original Quake formula:
	 *     save = ceil(absorption * damage)   <-- absorbed by armor
	 *     if (save > armor) save = armor     <-- can't drain more than we have
	 *     take = damage - save               <-- spills to HP
	 *
	 * For 100 HP + 100 green armor (absorption=0.3) hit by 50 damage:
	 *     save = 15, take = 35  -> HP=65, Armor=85.
	 *
	 * Pass InAbsorption = 0.0 (or InArmor = 0) to bypass armor entirely:
	 * the function reduces to OutHealth = InHealth - InDamage.
	 */
	static void ApplyArmorAbsorption(
		float InHealth, float InArmor, float InAbsorption, float InDamage,
		float& OutHealth, float& OutArmor);

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Maximum HP. SPEC section 1.1 player stats: Health (max) = 100. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	/**
	 * Live HP. Decremented exclusively by TakeDamage per SPEC section 1.5
	 * "no code outside TakeDamage mutates health directly". Protected so
	 * external readers go through GetHealth().
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health")
	float Health = 100.f;

	/**
	 * Trigger the screen-flash damage feedback. Phase 2 stub: writes the
	 * intensity into a dynamic material instance parameter on the post-
	 * process component if one exists, otherwise no-ops. The actual
	 * post-process material asset is wired up in a later phase per
	 * SPEC section 7.1; this hook exists now so the call site in
	 * TakeDamage doesn't have to grow later.
	 */
	void TriggerDamageFlash(float Intensity);

private:
	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
	void OnFirePressed(const struct FInputActionValue& Value);
	void OnWeaponSlotPressed(const struct FInputActionValue& Value, int32 SlotIndex);

	/**
	 * Spawn all weapons the GameInstance says the player owns, one actor
	 * per non-null slot in GameInstance->OwnedWeaponClasses. Called from
	 * BeginPlay. Auto-equips the lowest-index owned weapon.
	 */
	void SpawnOwnedWeapons();
};
