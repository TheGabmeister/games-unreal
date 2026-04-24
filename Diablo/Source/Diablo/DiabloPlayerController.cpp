#include "DiabloPlayerController.h"
#include "DiabloHero.h"
#include "DiabloEnemy.h"
#include "Diablo.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

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
		TargetEnemy = Enemy;
		UE_LOG(LogDiablo, Display, TEXT("Targeting enemy: %s"), *Enemy->GetName());
		UAIBlueprintHelperLibrary::SimpleMoveToActor(this, TargetEnemy);
		return;
	}

	TargetEnemy = nullptr;
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitResult.ImpactPoint);
}

void ADiabloPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!TargetEnemy || TargetEnemy->IsDead())
	{
		TargetEnemy = nullptr;
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	ADiabloHero* Hero = Cast<ADiabloHero>(ControlledPawn);
	if (!Hero)
	{
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
