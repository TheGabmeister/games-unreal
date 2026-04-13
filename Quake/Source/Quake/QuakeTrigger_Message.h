#pragma once

#include "CoreMinimal.h"
#include "QuakeTrigger.h"
#include "QuakeTrigger_Message.generated.h"

/**
 * Phase 8 message trigger per SPEC section 5.6. Displays Message on the
 * HUD for Duration seconds via AQuakeHUD::ShowMessage, then fires the
 * base class Targets chain.
 *
 * Message is FText so localization (Phase 15+) only changes the field,
 * not the plumbing.
 */
UCLASS()
class QUAKE_API AQuakeTrigger_Message : public AQuakeTrigger
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, Category = "Trigger|Message")
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger|Message", meta = (ClampMin = "0.1"))
	float Duration = 3.f;

	virtual void Activate(AActor* InInstigator) override;
};
