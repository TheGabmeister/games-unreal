

#include "Player/CBAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Player/CBPlayerCharacter.h"

void UCBAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	PlayerCharacter = Cast<ACBPlayerCharacter>(TryGetPawnOwner());
}

void UCBAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!PlayerCharacter.IsValid())
	{
		PlayerCharacter = Cast<ACBPlayerCharacter>(TryGetPawnOwner());
		if (!PlayerCharacter.IsValid())
		{
			return;
		}
	}

	const FVector Velocity = PlayerCharacter->GetVelocity();
	Speed = Velocity.Size2D();
	VerticalVelocity = Velocity.Z;
	bIsMoving = Speed > 10.0f;

	if (UCharacterMovementComponent* CMC = PlayerCharacter->GetCharacterMovement())
	{
		bIsFalling = CMC->IsFalling();
	}

	bIsSpinning = PlayerCharacter->IsSpinning();
}
