#include "DiabloPlayerController.h"
#include "DiabloHero.h"
#include "DiabloHUDWidget.h"
#include "DiabloEnemy.h"
#include "DroppedItem.h"
#include "Diablo.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Camera/PlayerCameraManager.h"

ADiabloPlayerController::ADiabloPlayerController()
{
	bShowMouseCursor = true;
}

void ADiabloPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	CreateHUD();
}

void ADiabloPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (HUDWidget)
	{
		if (ADiabloHero* Hero = Cast<ADiabloHero>(InPawn))
		{
			HUDWidget->InitForHero(Hero);
			HUDWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}

void ADiabloPlayerController::CreateHUD()
{
	if (!HUDWidgetClass)
	{
		UE_LOG(LogDiablo, Warning, TEXT("HUDWidgetClass is null — run Setup HUD in DiabloTools"));
		return;
	}

	HUDWidget = CreateWidget<UDiabloHUDWidget>(this, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport();

		if (ADiabloHero* Hero = Cast<ADiabloHero>(GetPawn()))
		{
			HUDWidget->InitForHero(Hero);
		}
	}
}

void ADiabloPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(ClickAction, ETriggerEvent::Started, this, &ADiabloPlayerController::OnClickStarted);
	}
}

void ADiabloPlayerController::OnClickStarted()
{
	ADiabloHero* Hero = Cast<ADiabloHero>(GetPawn());
	if (!Hero || Hero->IsDead())
	{
		return;
	}

	FHitResult HitResult;
	if (!GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), false, HitResult))
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	UE_LOG(LogDiablo, Display, TEXT("Click hit: %s (Component: %s)"),
		HitActor ? *HitActor->GetName() : TEXT("null"),
		HitResult.GetComponent() ? *HitResult.GetComponent()->GetName() : TEXT("null"));

	if (ADiabloEnemy* Enemy = Cast<ADiabloEnemy>(HitActor))
	{
		if (!Enemy->IsDead())
		{
			TargetEnemy = Enemy;
			TargetItem = nullptr;
			UE_LOG(LogDiablo, Display, TEXT("Targeting enemy: %s"), *Enemy->GetName());
			UAIBlueprintHelperLibrary::SimpleMoveToActor(this, TargetEnemy);
			return;
		}
	}

	if (ADroppedItem* Item = Cast<ADroppedItem>(HitActor))
	{
		TargetItem = Item;
		TargetEnemy = nullptr;
		UE_LOG(LogDiablo, Display, TEXT("Targeting item: %s"), *Item->GetName());
		UAIBlueprintHelperLibrary::SimpleMoveToActor(this, Item);
		return;
	}

	TargetEnemy = nullptr;
	TargetItem = nullptr;
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitResult.ImpactPoint);
}

void ADiabloPlayerController::OnHeroDeath()
{
	TargetEnemy = nullptr;
	TargetItem = nullptr;

	if (HUDWidget)
	{
		HUDWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	DisableInput(this);

	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(0.f, 1.f, 0.5f, FLinearColor::Black, false, true);
	}

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this,
		&ADiabloPlayerController::OnRespawnTimerExpired, 2.f, false);
}

void ADiabloPlayerController::OnRespawnTimerExpired()
{
	if (APawn* DeadPawn = GetPawn())
	{
		UnPossess();
		DeadPawn->Destroy();
	}

	if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
	{
		GM->RestartPlayer(this);
	}

	EnableInput(this);

	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(1.f, 0.f, 0.5f, FLinearColor::Black);
	}
}

void ADiabloPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ADiabloHero* Hero = Cast<ADiabloHero>(GetPawn());
	if (!Hero || Hero->IsDead())
	{
		TargetEnemy = nullptr;
		TargetItem = nullptr;
		return;
	}

	if (TargetItem && !IsValid(TargetItem))
	{
		TargetItem = nullptr;
	}

	if (TargetItem)
	{
		const float ItemDist = FVector::Dist(Hero->GetActorLocation(), TargetItem->GetActorLocation());
		if (ItemDist <= PickupRange)
		{
			StopMovement();
			Hero->GetCharacterMovement()->StopActiveMovement();
			TargetItem->OnPickedUp(Hero);
			TargetItem = nullptr;
		}
		return;
	}

	if (!TargetEnemy || TargetEnemy->IsDead())
	{
		TargetEnemy = nullptr;
		return;
	}

	const float Distance = FVector::Dist(Hero->GetActorLocation(), TargetEnemy->GetActorLocation());
	if (Distance <= AttackRange)
	{
		StopMovement();
		Hero->GetCharacterMovement()->StopActiveMovement();

		const FVector Direction = (TargetEnemy->GetActorLocation() - Hero->GetActorLocation()).GetSafeNormal2D();
		if (!Direction.IsNearlyZero())
		{
			Hero->SetActorRotation(Direction.Rotation());
		}

		if (!Hero->IsAttacking())
		{
			UE_LOG(LogDiablo, Display, TEXT("In range (%.0f <= %.0f), attacking"), Distance, AttackRange);
			Hero->AttackTarget = TargetEnemy;
			Hero->StartAttack();
		}
	}
}
