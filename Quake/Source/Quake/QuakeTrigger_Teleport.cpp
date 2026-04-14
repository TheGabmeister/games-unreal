#include "QuakeTrigger_Teleport.h"

#include "QuakeSoundManager.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeTeleport, Log, All);

void AQuakeTrigger_Teleport::Activate(AActor* InInstigator)
{
	if (!Destination)
	{
		UE_LOG(LogQuakeTeleport, Warning,
			TEXT("%s: Destination is null — authoring error."), *GetName());
		return;
	}

	ACharacter* Char = Cast<ACharacter>(InInstigator);
	if (!Char)
	{
		// Non-character instigators (e.g. a relay that fired this) are a
		// no-op for the teleport step — but still propagate to the chain.
		Super::Activate(InInstigator);
		return;
	}

	const FRotator NewRot = Destination->GetActorRotation();
	const FVector NewLoc = Destination->GetActorLocation();

	// Preserve velocity magnitude, rotate it to the destination's yaw.
	FVector NewVelocity = FVector::ZeroVector;
	if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
	{
		const float Speed = Move->Velocity.Size();
		NewVelocity = FRotator(0.f, NewRot.Yaw, 0.f).RotateVector(FVector::ForwardVector) * Speed;
		Move->Velocity = NewVelocity;
	}

	Char->SetActorLocationAndRotation(NewLoc, NewRot);
	if (AController* Ctrl = Char->GetController())
	{
		Ctrl->SetControlRotation(NewRot);
	}

	UQuakeSoundManager::PlaySoundEvent(this, EQuakeSoundEvent::Teleport, NewLoc);

	Super::Activate(InInstigator);
}
