// Fill out your copyright notice in the Description page of Project Settings.
#include "CBGameInstance.h"
#include "GameFramework/GameUserSettings.h"

void UCBGameInstance::Init()
{
	Super::Init();
	
	UGameUserSettings* GameUserSettings = GEngine->GetGameUserSettings();
	if (GameUserSettings)
	{
		GameUserSettings->SetScreenResolution(FIntPoint(1024, 768));
		GameUserSettings->SetFullscreenMode(EWindowMode::Windowed);
		GameUserSettings->ApplySettings(false);
	}
}

