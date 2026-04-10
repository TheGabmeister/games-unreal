#include "QuakeCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "InputModifiers.h"

AQuakeCharacter::AQuakeCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(35.f, 90.f);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = 600.f;
	Movement->BrakingDecelerationWalking = 1400.f;
	Movement->JumpZVelocity = 420.f;
	Movement->AirControl = 0.3f;
}

void AQuakeCharacter::SetupInputMappings()
{
	// Create Input Actions
	MoveAction = NewObject<UInputAction>(this, TEXT("IA_Move"));
	MoveAction->ValueType = EInputActionValueType::Axis2D;

	LookAction = NewObject<UInputAction>(this, TEXT("IA_Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;

	JumpAction = NewObject<UInputAction>(this, TEXT("IA_Jump"));
	JumpAction->ValueType = EInputActionValueType::Boolean;

	// Create Mapping Context and bind keys
	InputMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Default"));

	// W — forward (+Y): Swizzle to move value from X to Y axis
	FEnhancedActionKeyMapping& MapW = InputMappingContext->MapKey(MoveAction, EKeys::W);
	UInputModifierSwizzleAxis* SwizzleW = NewObject<UInputModifierSwizzleAxis>(this);
	SwizzleW->Order = EInputAxisSwizzle::YXZ;
	MapW.Modifiers.Add(SwizzleW);

	// S — backward (-Y): Swizzle + Negate
	FEnhancedActionKeyMapping& MapS = InputMappingContext->MapKey(MoveAction, EKeys::S);
	UInputModifierSwizzleAxis* SwizzleS = NewObject<UInputModifierSwizzleAxis>(this);
	SwizzleS->Order = EInputAxisSwizzle::YXZ;
	MapS.Modifiers.Add(SwizzleS);
	UInputModifierNegate* NegateS = NewObject<UInputModifierNegate>(this);
	MapS.Modifiers.Add(NegateS);

	// D — right (+X): no modifiers needed
	InputMappingContext->MapKey(MoveAction, EKeys::D);

	// A — left (-X): Negate
	FEnhancedActionKeyMapping& MapA = InputMappingContext->MapKey(MoveAction, EKeys::A);
	UInputModifierNegate* NegateA = NewObject<UInputModifierNegate>(this);
	MapA.Modifiers.Add(NegateA);

	// Mouse look
	FEnhancedActionKeyMapping& MapMouseX = InputMappingContext->MapKey(LookAction, EKeys::Mouse2D);

	// Jump
	InputMappingContext->MapKey(JumpAction, EKeys::SpaceBar);
}

void AQuakeCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetupInputMappings();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void AQuakeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AQuakeCharacter::Move);
	EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AQuakeCharacter::Look);
	EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
	EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
}

void AQuakeCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, Input.Y);
	AddMovementInput(RightDir, Input.X);
}

void AQuakeCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();

	AddControllerYawInput(Input.X * LookSensitivity);
	AddControllerPitchInput(-Input.Y * LookSensitivity);
}
