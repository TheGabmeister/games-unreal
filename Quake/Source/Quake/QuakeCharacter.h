#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "QuakeCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;

UCLASS()
class QUAKE_API AQuakeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AQuakeCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float LookSensitivity = 0.5f;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void SetupInputMappings();

	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);

	UPROPERTY()
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY()
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY()
	TObjectPtr<UInputAction> JumpAction;
};
