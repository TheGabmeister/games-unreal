// Fill out your copyright notice in the Description page of Project Settings.
#include "CrashBandicootGameInstance.h"
#include "GameFramework/GameUserSettings.h"

void UCrashBandicootGameInstance::Init()
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

