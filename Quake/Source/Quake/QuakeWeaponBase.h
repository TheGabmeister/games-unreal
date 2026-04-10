#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
	 * Returns true if the cooldown has elapsed since the last successful
	 * Fire. Pure check — does not consume the cooldown.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool CanFire() const;

	/**
	 * Public entry point. Checks the cooldown, calls Fire if ready, and
	 * arms the next cooldown window. Returns true iff Fire was actually
	 * invoked. Cooldown gating lives here so subclasses only have to
	 * implement the per-weapon hit/projectile logic.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool TryFire(AActor* InInstigator);

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
