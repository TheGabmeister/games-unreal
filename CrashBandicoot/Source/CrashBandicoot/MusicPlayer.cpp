
#include "MusicPlayer.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void UMusicPlayer::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	AudioComponent = nullptr;
	CurrentSound = nullptr;
}

void UMusicPlayer::Deinitialize()
{
	// Clean up
	if (AudioComponent)
	{
		AudioComponent->Stop();
		AudioComponent = nullptr;
	}
    
	CurrentSound = nullptr;
    
	Super::Deinitialize();
}

void UMusicPlayer::PlayMusic(USoundBase* Sound, float Volume, float StartTime)
{
	if (!Sound)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMusicPlayer::PlayMusic called with null Sound"));
		return;
	}
    
	// Stop any currently playing music first
	StopMusic();
    
	// Store the current sound
	CurrentSound = Sound;
    
	// Create a new audio component using UGameplayStatics
	// This ensures proper registration with the audio system
	AudioComponent = UGameplayStatics::CreateSound2D(this, Sound, Volume);
    
	if (AudioComponent)
	{
		if (StartTime > 0.0f)
		{
			AudioComponent->Play(StartTime);
		}
		else
		{
			AudioComponent->Play();
		}
	}
}

void UMusicPlayer::PauseMusic()
{
	if (AudioComponent && AudioComponent->IsPlaying())
	{
		AudioComponent->SetPaused(true);
	}
}

void UMusicPlayer::ResumeMusic()
{
	if (AudioComponent && AudioComponent->bIsPaused)
	{
		AudioComponent->SetPaused(false);
	}
}

void UMusicPlayer::StopMusic()
{
	if (AudioComponent)
	{
		AudioComponent->Stop();
		AudioComponent = nullptr;
	}
}

bool UMusicPlayer::IsPlaying() const
{
	return AudioComponent && AudioComponent->IsPlaying();
}