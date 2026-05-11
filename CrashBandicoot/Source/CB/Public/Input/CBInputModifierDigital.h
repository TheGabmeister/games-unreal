

#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "CBInputModifierDigital.generated.h"

/**
 * Normalizes analog stick input to full magnitude (1.0) when past the deadzone threshold.
 * Produces digital-feeling movement from an analog source.
 */
UCLASS(NotBlueprintable, MinimalAPI, meta = (DisplayName = "CB Digital Normalize"))
class UCBInputModifierDigital : public UInputModifier
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = Settings, meta = (ClampMin = "0", ClampMax = "1"))
	float DeadZoneThreshold = 0.2f;

protected:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;
};
