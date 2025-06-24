#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "MusicPlayer.generated.h"

/**
 * Subsystem for handling music playback that can be accessed from Blueprints
 */
UCLASS()
class CRASHBANDICOOT_API UMusicPlayer : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// Initialize the subsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	// Deinitialize the subsystem
	virtual void Deinitialize() override;
	
	/** Plays the specified sound with optional volume and start time */
	UFUNCTION(BlueprintCallable, Category = "Audio|Music")
	void PlayMusic(USoundBase* Sound, float Volume = 1.0f, float StartTime = 0.0f);
	
	/** Pauses the currently playing music */
	UFUNCTION(BlueprintCallable, Category = "Audio|Music")
	void PauseMusic();
	
	/** Resumes the paused music */
	UFUNCTION(BlueprintCallable, Category = "Audio|Music")
	void ResumeMusic();
	
	/** Stops the currently playing music */
	UFUNCTION(BlueprintCallable, Category = "Audio|Music")
	void StopMusic();
	
	/** Gets whether music is currently playing */
	UFUNCTION(BlueprintPure, Category = "Audio|Music")
	bool IsPlaying() const;
	
private:
	// Audio component used for music playback
	UPROPERTY()
	UAudioComponent* AudioComponent;
};
