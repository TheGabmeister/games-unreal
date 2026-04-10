#include "QuakePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"

void AQuakePlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetupInputMappings();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}
}

void AQuakePlayerController::SetupInputMappings()
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
	InputMappingContext->MapKey(LookAction, EKeys::Mouse2D);

	// Jump
	InputMappingContext->MapKey(JumpAction, EKeys::SpaceBar);
}
