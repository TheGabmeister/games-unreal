#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "QuakeGameUserSettings.generated.h"

/**
 * Phase 14 settings persistence per DESIGN 8 / ROADMAP. Wraps UE's
 * GameUserSettings so mouse sensitivity + master volume survive a restart
 * via `GameUserSettings.ini`. Read everywhere via `Get()` (which forwards
 * to `UGameUserSettings::GetGameUserSettings` and casts).
 *
 * To activate as the project's user settings class:
 *   `Config/DefaultEngine.ini` → `[/Script/Engine.Engine]`
 *   `GameUserSettingsClassName=/Script/Quake.QuakeGameUserSettings`
 */
UCLASS(Config = GameUserSettings, BlueprintType)
class QUAKE_API UQuakeGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UQuakeGameUserSettings();

	/**
	 * Convenience static finder. UE's `UGameUserSettings::GetGameUserSettings`
	 * returns the instance configured in DefaultEngine.ini; this casts it.
	 * Returns null if the project is still on the default UGameUserSettings.
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	static UQuakeGameUserSettings* Get();

	UFUNCTION(BlueprintCallable, Category = "Settings|Input")
	float GetMouseSensitivity() const { return MouseSensitivity; }

	/** Set + apply (does not save). Call SaveSettings() to persist to ini. */
	UFUNCTION(BlueprintCallable, Category = "Settings|Input")
	void SetMouseSensitivity(float NewValue);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMasterVolume(float NewValue);

	virtual void SetToDefaults() override;

private:
	/** SPEC default 0.5 — matches AQuakeCharacter::LookSensitivity initialiser. */
	UPROPERTY(Config)
	float MouseSensitivity = 0.5f;

	/** Linear gain 0..1. Phase 14 stub: stored only — no audio routing yet. */
	UPROPERTY(Config)
	float MasterVolume = 1.f;
};
