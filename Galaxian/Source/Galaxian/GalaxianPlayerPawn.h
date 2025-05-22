// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GalaxianPlayerPawn.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UArrowComponent;
class UPaperSpriteComponent;

UCLASS()
class GALAXIAN_API AGalaxianPlayerPawn : public APawn
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShootAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere)
	float Speed = 10.0f;

    UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> BulletBlueprint;

	// Arrow component to indicate forward direction
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UArrowComponent* ArrowComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UPaperSpriteComponent* SpriteComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	void Shoot();

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



public:	

	// Sets default values for this pawn's properties
	AGalaxianPlayerPawn();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
