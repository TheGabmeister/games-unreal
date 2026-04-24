#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DiabloHero.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS(Abstract)
class DIABLO_API ADiabloHero : public ACharacter
{
	GENERATED_BODY()

public:
	ADiabloHero();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;
};
