#include "DiabloAnimInstance.h"

void UDiabloAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (APawn* Owner = TryGetPawnOwner())
	{
		Speed = Owner->GetVelocity().Size();
	}
}
