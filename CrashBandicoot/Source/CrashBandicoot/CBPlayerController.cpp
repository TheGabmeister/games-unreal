// Fill out your copyright notice in the Description page of Project Settings.


#include "CBPlayerController.h"
#include "Blueprint/UserWidget.h"

void ACBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (GameplayWidget)
	{
		WidgetInstance = CreateWidget<UUserWidget>(this, GameplayWidget);
		if (WidgetInstance)
		{
			WidgetInstance->AddToViewport();
		}
	}
}