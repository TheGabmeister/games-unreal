// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CBPlayerController.generated.h"

class UUserWidget;

UCLASS()
class CRASHBANDICOOT_API ACBPlayerController : public APlayerController
{
	GENERATED_BODY()

	UPROPERTY()
	UUserWidget* WidgetInstance;
	
public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> GameplayWidget;
	
};
