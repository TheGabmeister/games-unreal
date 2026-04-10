#include "QuakeHUD.h"

#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

void AQuakeHUD::DrawHUD()
{
	Super::DrawHUD();

	APawn* P = GetOwningPawn();
	if (!P)
	{
		return;
	}

	const FVector V = P->GetVelocity();
	const float HorizontalSpeed = FVector(V.X, V.Y, 0.f).Size();

	FString ModeStr = TEXT("None");
	if (const ACharacter* Char = Cast<ACharacter>(P))
	{
		if (const UCharacterMovementComponent* CMC = Char->GetCharacterMovement())
		{
			// UEnum::GetValueAsString returns e.g. "EMovementMode::MOVE_Walking" —
			// chop the prefix for readability.
			ModeStr = UEnum::GetValueAsString(CMC->MovementMode);
			int32 ColonIdx = INDEX_NONE;
			if (ModeStr.FindLastChar(':', ColonIdx))
			{
				ModeStr = ModeStr.RightChop(ColonIdx + 1);
			}
		}
	}

	UFont* Font = GEngine ? GEngine->GetLargeFont() : nullptr;

	const FString LineSpeed = FString::Printf(TEXT("Speed: %6.0f"), HorizontalSpeed);
	const FString LineZ     = FString::Printf(TEXT("Z vel: %6.0f"), V.Z);
	const FString LineMode  = FString::Printf(TEXT("Mode : %s"), *ModeStr);

	DrawText(LineSpeed, FLinearColor::White, 20.f, 20.f, Font);
	DrawText(LineZ,     FLinearColor::White, 20.f, 38.f, Font);
	DrawText(LineMode,  FLinearColor::White, 20.f, 56.f, Font);
}
