

#include "Input/CBInputModifierDigital.h"
#include "EnhancedPlayerInput.h"

FInputActionValue UCBInputModifierDigital::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	FVector Value = CurrentValue.Get<FVector>();
	float Magnitude = Value.Size2D();

	if (Magnitude < DeadZoneThreshold)
	{
		return FInputActionValue(CurrentValue.GetValueType(), FVector::ZeroVector);
	}

	FVector2D Dir2D(Value.X, Value.Y);
	Dir2D.Normalize();
	return FInputActionValue(CurrentValue.GetValueType(), FVector(Dir2D.X, Dir2D.Y, Value.Z));
}
