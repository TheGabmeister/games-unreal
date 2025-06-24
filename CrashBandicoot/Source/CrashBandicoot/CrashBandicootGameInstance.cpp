// Fill out your copyright notice in the Description page of Project Settings.
#include "GameFramework/GameUserSettings.h"
#include "CrashBandicootGameInstance.h"

void UCrashBandicootGameInstance::Init()
{
	Super::Init();
    
	bool bIsFirstLaunch = false;
    
	// Check if it's the first launch using GConfig
	if (!GConfig->GetBool(TEXT("/Script/YourGame.GameSettings"), TEXT("bHasLaunchedBefore"), bIsFirstLaunch, GGameIni))
	{
		// No entry found, this is the first launch
		bIsFirstLaunch = true;
        
		// Save the flag to config
		GConfig->SetBool(TEXT("/Script/YourGame.GameSettings"), TEXT("bHasLaunchedBefore"), true, GGameIni);
		GConfig->Flush(false, GGameIni);
        
		UE_LOG(LogTemp, Display, TEXT("This is the first time launching the game!"));
        
		// Additional first-time setup...
	}
    
	// Your existing settings code
	UGameUserSettings* GameUserSettings = GEngine->GetGameUserSettings();
	if (GameUserSettings)
	{
		GameUserSettings->SetScreenResolution(FIntPoint(1024, 768));
		GameUserSettings->SetFullscreenMode(EWindowMode::Windowed);
		GameUserSettings->ApplySettings(false);
	}
}

