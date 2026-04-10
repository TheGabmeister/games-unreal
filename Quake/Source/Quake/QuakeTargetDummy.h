#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuakeTargetDummy.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;

/**
 * Test-only target actor for Phase 2 damage-pipeline verification.
 *
 * A static cube with a Health field, a TakeDamage override that decrements
 * health, and a UTextRenderComponent floating above it that displays the
 * current HP. The whole point is to provide a visible, hittable target the
 * player can swing the Axe at on PhysSandbox to verify the damage pipeline
 * works end-to-end before any real enemies exist.
 *
 * Not an enemy — no AI, no pawn, no movement. When Health reaches zero the
 * actor logs a death message and disables further damage; it does not
 * destroy itself so the post-mortem visual stays visible during testing.
 *
 * SPEC Phase 2 manual verification: "Swing Axe at target dummy in
 * PhysSandbox. See HP text decrease by 20 per swing."
 */
UCLASS()
class QUAKE_API AQuakeTargetDummy : public AActor
{
	GENERATED_BODY()

public:
	AQuakeTargetDummy();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target Dummy")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target Dummy")
	TObjectPtr<UTextRenderComponent> HealthText;

	/** Starting and current HP. Set in the editor per-instance for varied tests. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target Dummy", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Target Dummy")
	float Health = 100.f;

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

private:
	void UpdateHealthText();
};
