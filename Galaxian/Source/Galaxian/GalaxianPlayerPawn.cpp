// Fill out your copyright notice in the Description page of Project Settings.


#include "GalaxianPlayerPawn.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "PaperSpriteComponent.h"

// Sets default values
AGalaxianPlayerPawn::AGalaxianPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	SpriteComp = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComp"));
	SpriteComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AGalaxianPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGalaxianPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGalaxianPlayerPawn::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Only move along the X axis (left/right)
		FVector DeltaLocation = FVector(MovementVector.X, 0.f, 0.f);
		AddActorWorldOffset(DeltaLocation * Speed, true);
	}
}



// In your .cpp file, implement Shoot:
void AGalaxianPlayerPawn::Shoot()
{
    if (BulletBlueprint)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();

        GetWorld()->SpawnActor<AActor>(BulletBlueprint, GetActorLocation(), GetActorRotation(), SpawnParams);
    }
}

void AGalaxianPlayerPawn::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AGalaxianPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AGalaxianPlayerPawn::Shoot);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGalaxianPlayerPawn::Move);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. "), *GetNameSafe(this));
	}
}