


#include "Audio/CBAudioSubsystem.h"
#include "AudioDevice.h"
#include "Components/AudioComponent.h"
#include "Engine/WorldInitializationValues.h"
#include "Game/CBGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "LoadingScreenManager.h"
#include "PlatformFeatures.h"
#include "Settings/CBDeveloperSettings.h"
#include "Settings/CBGameUserSettings.h"
#include "Settings/CBWorldSettings.h"

DEFINE_LOG_CATEGORY(LogCBAudioSubsystem); 

void UCBAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection); 

	// Ask the loading screen to notify us when its visibility changes
	TObjectPtr<ULoadingScreenManager> LoadingScreenManager = Collection.InitializeDependency<ULoadingScreenManager>();
	LoadingScreenManager->OnLoadingScreenVisibilityChangedDelegate().AddUObject(this, &UCBAudioSubsystem::LoadingScreenVisibilityChanged);

	// Get a reference to our game instance 
	CBGameInstance = Cast<UCBGameInstance>(GetGameInstance()); 

	// Get the class default object for CB Developer Settings 
	CurrentDeveloperSettings = GetDefault<UCBDeveloperSettings>();

	// Load default audio classes from the CB Developer Project Settings 
	// Note that we're using IsNull here to check if the soft object pointer path is null
	// IsValid, checks an already loaded object 

	// Default Sound Mixer
	if (!CurrentDeveloperSettings->DefaultSoundMixModifier.IsNull())
	{
		DefaultSoundMixModifier = CurrentDeveloperSettings->DefaultSoundMixModifier.LoadSynchronous(); 
	}
	else
	{
		UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::Initialize DefaultSoundMixModifier is invalid, check your CB Developer settings."));
	}

	// Main Sound 
	if (!CurrentDeveloperSettings->MainSoundClass.IsNull())
	{
		MainSoundClass = CurrentDeveloperSettings->MainSoundClass.LoadSynchronous();
	}
	else
	{
		UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::Initialize MainSoundClass is invalid, check your CB Developer settings."));
	}

	// Music 
	if (!CurrentDeveloperSettings->MusicSoundClass.IsNull())
	{
		MusicSoundClass = CurrentDeveloperSettings->MusicSoundClass.LoadSynchronous();
	}
	else
	{
		UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::Initialize MusicSoundClass is invalid, check your CB Developer settings."));
	}

	// SFX 
	if (!CurrentDeveloperSettings->SFXSoundClass.IsNull())
	{
		SFXSoundClass = CurrentDeveloperSettings->SFXSoundClass.LoadSynchronous();
	}
	else
	{
		UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::Initialize SFXSoundClass is invalid, check your CB Developer settings."));
	}

	// Load the (optional) default music for the game from the CB Developer Project Settings 
	if (!CurrentDeveloperSettings->LevelCompleteMusic.IsNull())
	{
		LevelCompleteMusic = CurrentDeveloperSettings->LevelCompleteMusic.LoadSynchronous(); 
	}

	if (!CurrentDeveloperSettings->LevelFailMusic.IsNull())
	{
		LevelFailMusic = CurrentDeveloperSettings->LevelFailMusic.LoadSynchronous(); 
	}

	// Bind to world initialization so that the audio subsystem knows about the world state (BeginPlay) 
	PostWorldCreationHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UCBAudioSubsystem::WorldInitialization);
}

void UCBAudioSubsystem::SaveAudioSettings()
{
	if (CurrentGameSettings)
	{
		CurrentGameSettings->SaveSettings();
	}
	else
	{
		UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::SaveAudioSettings CurrentGameSettings is nullptr."));
		return;
	}
}

void UCBAudioSubsystem::SetMainVolume(float NewVolume, float FadeIn)
{
	if (CurrentGameSettings)
	{
		CurrentGameSettings->MainVolume = NewVolume;

		ApplyVolumeChangeToMix(MainSoundClass, CurrentGameSettings->MainVolume, FadeIn);
	}
	else
	{
		UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::SetMainVolume CurrentGameSettings is nullptr."));
		return;
	}
}

void UCBAudioSubsystem::SetMusicVolume(float NewVolume, float FadeIn)
{
	if (CurrentGameSettings)
	{
		CurrentGameSettings->MusicVolume = NewVolume;

		ApplyVolumeChangeToMix(MusicSoundClass, CurrentGameSettings->MusicVolume, FadeIn);
	}
	else
	{
		UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::SetMusicVolume CurrentGameSettings is nullptr."));
		return;
	}
}

void UCBAudioSubsystem::SetSFXVolume(float NewVolume, float FadeIn)
{
	if (CurrentGameSettings)
	{
		CurrentGameSettings->SFXVolume = NewVolume;

		ApplyVolumeChangeToMix(SFXSoundClass, CurrentGameSettings->SFXVolume, FadeIn);
	}
	else
	{
		UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::SetSFXVolume CurrentGameSettings is nullptr."));
		return;
	}
}

float UCBAudioSubsystem::GetMainVolume()
{
	if (CurrentGameSettings)
	{
		return CurrentGameSettings->MainVolume;
	}
	
	UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::GetMainVolume CurrentGameSettings is nullptr."));
	return 1.0f; 
}

float UCBAudioSubsystem::GetMusicVolume()
{
	if (CurrentGameSettings)
	{
		return CurrentGameSettings->MusicVolume;
	}

	UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::GetMusicVolume CurrentGameSettings is nullptr."));
	return 1.0f;
}

float UCBAudioSubsystem::GetSFXVolume()
{
	if (CurrentGameSettings)
	{
		return CurrentGameSettings->SFXVolume;
	}

	UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::GetSFXVolume CurrentGameSettings is nullptr."));
	return 1.0f;
}

bool UCBAudioSubsystem::WorldMusicInitialized()
{
	if (!IsValid(WorldMusicPlayer))
	{
		return false; 
	}

	return true;
}

void UCBAudioSubsystem::PlaySoundAsWorldMusic(USoundBase* Music)
{
	PlayWorldMusic(Music); 
}

void UCBAudioSubsystem::ApplyVolumeChangeToMix(USoundClass* TargetSoundClass, float Volume, float FadeIn)
{
	// Get the audio device, apply the override to the mix, push modifier update 
	if (FAudioDeviceHandle AudioDevice = CurrentWorld->GetAudioDevice())
	{
		AudioDevice->SetSoundMixClassOverride(
			DefaultSoundMixModifier, /* Sound Mix Modifier */
			TargetSoundClass, /* Sound Class */
			Volume, /* Volume Multiplier*/
			1.0f, /* Pitch Multiplier */
			FadeIn, /* Fade In Time */
			true /* Apply To Children */
		);
		AudioDevice->PushSoundMixModifier(DefaultSoundMixModifier);
	}
}

void UCBAudioSubsystem::WorldInitialization(UWorld* World, const FWorldInitializationValues IVS)
{
	if (World)
	{
		// The world has been initialized so now we can bind to BeginPlay of the world. 
		// Here, we bind our world begin play function to this delegate. 
		CurrentWorld = World; 
		OnWorldBeginPlayHandle = CurrentWorld->OnWorldBeginPlay.AddUObject(this, &UCBAudioSubsystem::WorldBeginPlay);

		// Store the current world settings for pulling relevant audio data
		CurrentWorldSettings = Cast<ACBWorldSettings>(CurrentWorld->GetWorldSettings());

		if (!CurrentWorldSettings)
		{
			UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::WorldInitialization unable to get CB world settings. Check world %s"), *CurrentWorld->GetName());
			return; 
		}
	}
}

void UCBAudioSubsystem::WorldBeginPlay()
{
	// Get the game user settings
	CurrentGameSettings = GetCBGameSettings();

	// Apply sound settings to world audio device 
	UpdateMixersFromAudioData(); 

	// In editor, we want to play the music on world begin play
#if WITH_EDITOR
	// If the loading screen is held, the loading screen will notify us via LoadingScreenVisibilityChanged
	// If we are changing levels with no loading screen, just start the music
	if (IsValid(CBGameInstance) && !CBGameInstance->ShouldHoldLoadingScreen())
	{
		StartDefaultWorldMusic();
	}
#endif
	
}

void UCBAudioSubsystem::LoadingScreenVisibilityChanged(bool bVisible)
{
	if (!bVisible)
	{
		// Now that the loading screen is no longer visible, play the world music 
		StartDefaultWorldMusic();
	}
	else
	{
		// When a loading screen has popped up, stop any world music 
		if (IsValid(WorldMusicPlayer))
		{
			WorldMusicPlayer->Stop(); 
		}
	}
}

void UCBAudioSubsystem::UpdateMixersFromAudioData()
{
	if (IsValid(CurrentGameSettings))
	{
		// Apply audio data settings to mixer 
		ApplyVolumeChangeToMix(MainSoundClass, CurrentGameSettings->MainVolume, 0.0f);
		ApplyVolumeChangeToMix(MusicSoundClass, CurrentGameSettings->MusicVolume, 0.0f);
		ApplyVolumeChangeToMix(SFXSoundClass, CurrentGameSettings->SFXVolume, 0.0f);
	}
}

void UCBAudioSubsystem::StartDefaultWorldMusic()
{
	if (!IsValid(CurrentWorld))
	{
		UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::StartDefaultWorldMusic CurrentWorld is null"));
		return;
	}

	if (!IsValid(CurrentWorldSettings))
	{
		UE_LOG(LogCBAudioSubsystem, Error, TEXT("UCBAudioSubsystem::StartDefaultWorldMusic CurrentWorldSettings is null"));
		return;
	}

	if (!IsValid(CurrentWorldSettings->WorldMusic))
	{
		UE_LOG(LogCBAudioSubsystem, Warning, TEXT("UCBAudioSubsystem::StartDefaultWorldMusic WorldMusic is null. Check your world settings setup %s"), *CurrentWorld->GetName());
		return;
	}

	PlayWorldMusic(CurrentWorldSettings->WorldMusic); 
}

void UCBAudioSubsystem::PlayWorldMusic(USoundBase* Music)
{
	if (!IsValid(Music))
	{
		UE_LOG(LogCBAudioSubsystem, Warning, TEXT("UCBAudioSubsystem::PlayWorldMusic, Music parameter is null.")); 
		return; 
	}

	if (!IsValid(WorldMusicPlayer))
	{
		// Spawn our audio component 
		WorldMusicPlayer = UGameplayStatics::CreateSound2D(
			CurrentWorld, /* World Object */
			Music, /* USoundBase* */
			1.0f, /* Volume Multiplier */
			1.0f, /* Pitch Multiplier */
			0.0f, /* Start Time */
			nullptr, /* Concurrency Settings */
			true, /* Persist across level transition */
			false); /* Auto Destroy */

		if (CurrentGameSettings)
		{
			// Note that the reason we don't set the multiplier directly on the audio component is because 
			// the audio data multiplier is applied to the mix the sound is apart of, not the player itself. 
			SetMusicVolume(CurrentGameSettings->MusicVolume);
		}
	}
	
	if (WorldMusicPlayer->bIsPaused)
	{
		WorldMusicPlayer->SetPaused(false);
	}

	if (WorldMusicPlayer->IsPlaying())
	{
		WorldMusicPlayer->Stop();
	}

	// Switch the sound 
	WorldMusicPlayer->Sound = Music;

	// Play the music from the beginning 
	WorldMusicPlayer->Play();
}

void UCBAudioSubsystem::TogglePauseWorldMusic()
{
	if (!IsValid(WorldMusicPlayer))
	{
		return; 
	}

	// Toggle pause of the music player 
	WorldMusicPlayer->SetPaused(!WorldMusicPlayer->bIsPaused);
}

void UCBAudioSubsystem::StartVictoryMusic()
{
	if (!IsValid(WorldMusicPlayer))
	{
		return;
	}

	WorldMusicPlayer->Stop();

	if (!IsValid(LevelCompleteMusic))
	{
		// No music found so we'll just stop the music player and return 
		return;
	}

	// Set the new sound and play 
	WorldMusicPlayer->SetSound(LevelCompleteMusic); 
	WorldMusicPlayer->Play(); 
}

void UCBAudioSubsystem::StartGameOverMusic()
{
	if (!IsValid(WorldMusicPlayer))
	{
		return; 
	}

	WorldMusicPlayer->Stop();

	if (!IsValid(LevelFailMusic))
	{
		// No music found so we'll just stop the music player and return 
		return;
	}

	// Set the new sound and play 
	WorldMusicPlayer->SetSound(LevelFailMusic);
	WorldMusicPlayer->Play();
}

void UCBAudioSubsystem::PlayInvincibilityMusic()
{
	if (IsValid(InvincibilityMusic))
	{
		PlayWorldMusic(InvincibilityMusic);
	}
}

void UCBAudioSubsystem::PlayWorldMusic()
{
	if (IsValid(CurrentWorldSettings) && IsValid(CurrentWorldSettings->WorldMusic))
	{
		PlayWorldMusic(CurrentWorldSettings->WorldMusic);
	}
}

TObjectPtr<UCBGameUserSettings> UCBAudioSubsystem::GetCBGameSettings()
{
	if (GEngine)
	{
		return Cast<UCBGameUserSettings>(GEngine->GetGameUserSettings());
	}

	return nullptr;
}
