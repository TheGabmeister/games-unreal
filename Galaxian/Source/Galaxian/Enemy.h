// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseActor2D.h"
#include "Enemy.generated.h"

UCLASS()
class GALAXIAN_API AEnemy : public ABaseActor2D
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	float Score = 0;

public:	
	// Sets default values for this actor's properties
	AEnemy();

};
