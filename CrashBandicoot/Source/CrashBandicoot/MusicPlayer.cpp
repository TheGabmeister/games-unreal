// Fill out your copyright notice in the Description page of Project Settings.

#include "MusicPlayer.h"
#include "Kismet/GameplayStatics.h"

void UMusicPlayer::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// Create audio component for music playback
	AudioComponent = NewObject<UAudioComponent>(this);
	AudioComponent->bAutoActivate = false;
	AudioComponent->bAutoDestroy = false;
	AudioComponent->SetVolumeMultiplier(1.0f);

	if (GetWorld())
	{
		AudioComponent->RegisterComponent();
	}

}

void UMusicPlayer::Deinitialize()
{
	// Clean up the audio component if needed
	if (AudioComponent)
	{
		AudioComponent->Stop();
		AudioComponent = nullptr;
	}
	
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
	if (AudioComponent)
	{
		AudioComponent->Stop();
		AudioComponent->SetSound(Sound);
		AudioComponent->SetVolumeMultiplier(Volume);
		AudioComponent->Play(StartTime);
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
	}
}

bool UMusicPlayer::IsPlaying() const
{
	return AudioComponent && AudioComponent->IsPlaying();
}