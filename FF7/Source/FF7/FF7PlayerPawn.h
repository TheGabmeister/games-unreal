// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FF7CharacterBase.h"
#include "FF7PlayerPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;

/**
 * Player-controlled pawn (SPEC §2.3). Adds a fixed top-down camera rig
 * (no orbit, no pitch follow) and a minimal floating movement component
 * so controller input translates into actual movement.
 */
UCLASS()
class FF7_API AFF7PlayerPawn : public AFF7CharacterBase
{
	GENERATED_BODY()

public:
	AFF7PlayerPawn();

	USpringArmComponent* GetSpringArm() const { return SpringArm; }
	UCameraComponent* GetCamera() const { return Camera; }
	UFloatingPawnMovement* GetMovement() const { return Movement; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FF7|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FF7|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FF7|Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFloatingPawnMovement> Movement;
};
