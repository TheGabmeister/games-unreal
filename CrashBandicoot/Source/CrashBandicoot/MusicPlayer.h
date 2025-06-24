#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MusicPlayer.generated.h"

/**
 * Subsystem for handling music playback that can be accessed from Blueprints
 */
UCLASS()
class CRASHBANDICOOT_API UMusicPlayer : public UGameInstanceSubsystem
{
	GENERATED_BODY()
    
public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category = "MusicPlayer")
	void PlayMusic(USoundBase* Sound, float Volume = 1.0f, float StartTime = 0.0f);
	
	UFUNCTION(BlueprintCallable, Category = "MusicPlayer")
	void PauseMusic();
	
	UFUNCTION(BlueprintCallable, Category = "MusicPlayer")
	void ResumeMusic();
	
	UFUNCTION(BlueprintCallable, Category = "MusicPlayer")
	void StopMusic();
	
	UFUNCTION(BlueprintPure, Category = "MusicPlayer")
	bool IsPlaying() const;
    
private:
	
	UPROPERTY()
	UAudioComponent* AudioComponent;
	
	UPROPERTY()
	USoundBase* CurrentSound;
};