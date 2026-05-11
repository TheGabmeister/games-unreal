#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "LoadingProcessInterface.h"

#include "CBGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWindowFocusChanged, bool, bIsFocused);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLivesChanged, int32, NewLives);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWumpaChanged, int32, NewCount);

/**
 * The GameInstance: high-level manager object for an instance of the running game.
 * Spawned at game creation and not destroyed until game instance is shut down.
 * Running as a standalone game, there will be one of these.
 * Running in PIE (play-in-editor) will generate one of these per PIE instance.
 */

/**
* The Loading Process Interface is used for things that might cause loading to happen which requires a loading screen to be displayed.
* In CB's case, this is the startup UMG boot splash videos. 
*/

UCLASS(Abstract, Blueprintable)
class UCBGameInstance : public UGameInstance, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:
	// Returns true if the boot splash/logo train has been played in this session
	UFUNCTION(BlueprintPure)
	bool HasPlayedBootSplash() const;

	// Records whether or not the boot splash/logo train has been played this session
	UFUNCTION(BlueprintCallable)
	void SetHasPlayedBootSplash(bool bPlayed);

	// Allows the loading screen to be held while something happens (e.g. playing a video)
	// NOTE: be sure to call HoldLoadingScreen(false) when you're done!
	UFUNCTION(BlueprintCallable)
	void HoldLoadingScreen(bool bHold);

	// Returns true if the loading screen has been requested to be held
	UFUNCTION(BlueprintPure)
	bool ShouldHoldLoadingScreen() const;

	//--- GameInstance overrides
	void Init() override;
#if WITH_EDITOR
	FGameInstancePIEResult InitializeForPlayInEditor(int32 PIEInstanceIndex, const FGameInstancePIEParameters& Params) override;
#endif
	//--- End GameInstance

	//--- ILoadingProcessInterface overrides
	bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//--- End ILoadingProcessInterface

	// --- Lives System ---

	UFUNCTION(BlueprintCallable, Category = "CB")
	void AddLife();

	UFUNCTION(BlueprintCallable, Category = "CB")
	bool LoseLife();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CB")
	int32 GetLives() const { return Lives; }

	UFUNCTION(BlueprintCallable, Category = "CB")
	void ResetLives();

	UPROPERTY(BlueprintAssignable, Category = "CB")
	FOnLivesChanged OnLivesChanged;

	// --- Wumpa System ---

	UFUNCTION(BlueprintCallable, Category = "CB")
	void AddWumpa(int32 Amount);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CB")
	int32 GetWumpaCount() const { return WumpaCount; }

	UPROPERTY(BlueprintAssignable, Category = "CB")
	FOnWumpaChanged OnWumpaChanged;

	// --- Aku Aku Persistence ---

	UPROPERTY(BlueprintReadWrite, Category = "CB")
	uint8 StoredAkuAkuState = 0;

public:

	UPROPERTY(BlueprintAssignable)
	FOnWindowFocusChanged OnWindowFocusChanged;

private:
	void WindowFocusChanged(bool bIsFocused);

	bool bHasPlayedBootSplash = false;
	bool bHoldLoadingScreen = false;

	UPROPERTY(VisibleAnywhere, Category = "CB")
	int32 Lives = 4;

	UPROPERTY(VisibleAnywhere, Category = "CB")
	int32 WumpaCount = 0;

	static constexpr int32 MaxLives = 99;
	static constexpr int32 StartingLives = 4;
};
