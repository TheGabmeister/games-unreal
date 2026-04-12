#include "QuakeHUD.h"

#include "SQuakeHUDOverlay.h"

#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Widgets/SWidget.h"

void AQuakeHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !GEngine || !GEngine->GameViewport)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	// Pass the controller, not the current pawn — the overlay re-resolves
	// the pawn on every paint so the HUD stays bound to the live body
	// after a death-restart swaps the pawn out from under the controller.
	OverlayWidget = SNew(SQuakeHUDOverlay)
		.OwningPlayerController(PC);

	GEngine->GameViewport->AddViewportWidgetForPlayer(
		LocalPlayer,
		OverlayWidget.ToSharedRef(),
		/*ZOrder=*/10);
}

void AQuakeHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OverlayWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		if (APlayerController* PC = GetOwningPlayerController())
		{
			if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
			{
				GEngine->GameViewport->RemoveViewportWidgetForPlayer(
					LocalPlayer,
					OverlayWidget.ToSharedRef());
			}
		}
		OverlayWidget.Reset();
	}
	Super::EndPlay(EndPlayReason);
}

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
