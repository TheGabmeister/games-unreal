#pragma once

#include "CoreMinimal.h"
#include "QuakeTrigger.h"
#include "QuakeTrigger_Secret.generated.h"

/**
 * Phase 8 secret trigger per SPEC section 5.8. On first activation, credits
 * a secret on the PlayerState's SecretsFound count and shows "A secret area!"
 * on the HUD. One-shot — subsequent overlaps are ignored so players don't
 * re-credit the same secret by walking out and back in.
 *
 * The PlayerState secrets counter is wired in Phase 9 (Stats). Until then,
 * this trigger just logs the event and shows the HUD message; the counter
 * increment is a TODO commented inline.
 */
UCLASS()
class QUAKE_API AQuakeTrigger_Secret : public AQuakeTrigger
{
	GENERATED_BODY()

public:
	virtual void Activate(AActor* InInstigator) override;

private:
	bool bCredited = false;
};
