#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuakeAmmoType.h"
#include "QuakeWeaponBase.generated.h"

class UStaticMeshComponent;

/**
 * Base class for every Quake weapon. Owns the viewmodel root, the per-shot
 * cooldown timer, and the abstract Fire() entry point. Concrete weapons
 * (Axe, Shotgun, Nailgun, etc.) subclass this and override Fire to do
 * their hitscan trace, projectile spawn, or melee swing.
 *
 * Weapons are AActors attached to the firing pawn (per SPEC section 1.4):
 * the character's BeginPlay spawns and attaches them based on the
 * GameInstance inventory, and the character's input binding routes the
 * Fire input to CurrentWeapon->TryFire.
 *
 * Stats (damage, RoF, range, etc.) live in the C++ subclass constructor as
 * UPROPERTY defaults per SPEC section 2 — thin BP subclasses (BP_Weapon_*)
 * only fill in asset slots (mesh, material, sound). Zero nodes in the BP
 * event graph.
 *
 * Cooldown enforcement is centralized here (CanFire / TryFire) so every
 * subclass gets it for free without re-implementing the timer.
 */
UCLASS(Abstract)
class QUAKE_API AQuakeWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AQuakeWeaponBase();

	/** Viewmodel root. Subclasses / BP fill in the mesh asset. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> ViewModelMesh;

	/**
	 * Rate of fire in shots per second. SPEC section 2.0 weapon table:
	 * Axe = 2/sec, Shotgun = 1.5/sec, etc. The reciprocal is the cooldown
	 * between shots in seconds. Set in the subclass constructor.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0.01"))
	float RateOfFire = 2.f;

	/**
	 * Display name used by the HUD weapon readout. Set in subclass
	 * constructor (e.g., "Axe", "Shotgun"). The HUD reads this in the
	 * Slate paint path so no asset lookup is required at runtime.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FText DisplayName;

	/**
	 * Ammo type consumed per shot. EQuakeAmmoType::None means the weapon
	 * fires without consuming ammo (the Axe). TryFire uses this as the
	 * key for the GameInstance ammo check.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
	EQuakeAmmoType AmmoType = EQuakeAmmoType::None;

	/**
	 * Ammo consumed per successful shot. 0 when AmmoType is None; 1 for
	 * Shotgun / Nailgun / Rocket Launcher; 2 for Double-Barrel Shotgun and
	 * Super Nailgun.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	int32 AmmoPerShot = 0;

	/**
	 * Returns true if the cooldown has elapsed since the last successful
	 * Fire. Pure check — does not consume the cooldown.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool CanFire() const;

	/**
	 * Public entry point. Runs the cooldown gate, consumes ammo (if the
	 * weapon has an ammo type), and calls Fire if both pass. On empty
	 * ammo it plays a "click" stub and arms the cooldown anyway so held
	 * fire spams click at the weapon's RoF rather than at tick rate.
	 * Returns true iff Fire was actually invoked.
	 *
	 * The ammo check reads from the Instigator's UQuakeGameInstance so
	 * weapons don't hold local ammo state — inventory ownership stays on
	 * the GameInstance per CLAUDE.md "Architecture: State Ownership".
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool TryFire(AActor* InInstigator);

protected:
	/**
	 * "Click" feedback when firing on empty. Base stub logs and emits a
	 * hearing noise event so AI still hears the failed fire attempt (SPEC
	 * 3.3 wants enemies to react to the player making noise, and a dry
	 * fire is still a meaningful audible cue in Quake). Subclasses may
	 * override to play the weapon-specific click asset when sound lands.
	 */
	virtual void PlayEmptyClick(AActor* InInstigator);

protected:
	/**
	 * Subclass hook: do the actual hit logic (trace, projectile spawn,
	 * splash, etc.). Called by TryFire after the cooldown gate. Pure
	 * virtual — every concrete weapon must implement it. Uses
	 * PURE_VIRTUAL (rather than `= 0`) because UCLASS reflection still
	 * needs the CDO to be constructible; the macro emits a crashing body
	 * that fires only if the base is somehow called directly.
	 */
	virtual void Fire(AActor* InInstigator) PURE_VIRTUAL(AQuakeWeaponBase::Fire, );

private:
	/** World seconds at the most recent successful Fire. -1 = never fired. */
	double LastFireWorldTime = -1.0;
};
