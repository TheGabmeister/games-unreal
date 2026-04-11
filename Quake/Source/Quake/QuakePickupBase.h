#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuakePickupBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class AQuakeCharacter;

/**
 * Phase 4 pickup base. One concrete subclass per pickup category
 * (AQuakePickup_Health, AQuakePickup_Ammo, and — in later phases —
 * AQuakePickup_Armor / AQuakePickup_Powerup / AQuakePickup_Key).
 *
 * Per SPEC section 4:
 *   - USphereComponent (overlap-only, default 64 u radius) for detection.
 *   - UStaticMeshComponent for the visual primitive.
 *   - UPointLightComponent for the colored glow.
 *   - OnComponentBeginOverlap handler bound in BeginPlay.
 *   - Handler casts overlap actor to AQuakeCharacter, runs CanBeConsumedBy,
 *     applies the effect, destroys.
 *
 * Per SPEC 1.6 rule 4 ("pickup overlap is player-only"), enemies can
 * trigger the overlap but the cast-to-AQuakeCharacter bail keeps them
 * from consuming pickups.
 *
 * Subclasses override CanBeConsumedBy (optional — default returns true)
 * and ApplyPickupEffectTo (PURE_VIRTUAL — same CDO gotcha as
 * AQuakeWeaponBase::Fire, use the engine macro).
 */
UCLASS(Abstract)
class QUAKE_API AQuakePickupBase : public AActor
{
	GENERATED_BODY()

public:
	AQuakePickupBase();

	/** Overlap trigger volume. Subclasses can tune Radius in their ctor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USphereComponent> PickupSphere;

	/** Visual primitive; BP subclass fills in the mesh + material slots. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	/** Colored glow; BP subclass tunes color / intensity. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UPointLightComponent> GlowLight;

	/**
	 * Should this pickup consume itself for the given character? Default
	 * is "yes, always". Health pickups override to check HP < Max unless
	 * they're overcharge-type.
	 */
	virtual bool CanBeConsumedBy(AQuakeCharacter* Character) const { return Character != nullptr; }

	/**
	 * Apply the pickup effect. Pure C++ virtual — subclasses MUST
	 * implement. PURE_VIRTUAL macro is required here (not `= 0`) so the
	 * abstract CDO stays constructible.
	 */
	virtual void ApplyPickupEffectTo(AQuakeCharacter* Character) PURE_VIRTUAL(AQuakePickupBase::ApplyPickupEffectTo, );

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnPickupBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
