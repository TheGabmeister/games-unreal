#pragma once

#include "CoreMinimal.h"
#include "QuakeSoundEvent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuakeSoundManager.generated.h"

class UDataTable;
class USoundBase;

/**
 * Phase 14 / DESIGN 8.1 audio entry point. Every gameplay sound event flows
 * through `PlaySound` here; rows in `DT_SoundEvents` decide whether an asset
 * actually plays. Subsystems can NOT be Blueprint-subclassed, so the
 * DataTable reference lives on `UQuakeGameInstance::SoundEventTable` (which
 * has a BP subclass for asset assignment) — the manager pulls it on first
 * use and caches.
 *
 * No-asset rows make `PlaySound` a no-op + Verbose log. Phase 14 ships
 * with every row's `Sound = nullptr` so the project can launch + complete
 * its automated tests before any audio assets exist.
 */
UCLASS()
class QUAKE_API UQuakeSoundManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Play a 3D sound for the given event at Location. If the table row is
	 * missing or its Sound asset is null, no-op (logs at Verbose).
	 *
	 * Static convenience helper finds the manager off any WorldContext,
	 * so call sites don't need to plumb `GetGameInstance()` themselves —
	 * `UQuakeSoundManager::PlaySound(this, EQuakeSoundEvent::PlayerJump, GetActorLocation())`
	 * is the canonical pattern.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySound(EQuakeSoundEvent Event, FVector Location);

	/**
	 * Convenience for HUD / 2D sounds (no spatial position). Forwards to
	 * `PlaySound2D` on the row's asset.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySound2D(EQuakeSoundEvent Event);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayMusic(EQuakeMusicTrack Track);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopMusic();

	/** Static finder. Returns null if there is no GameInstance / subsystem yet. */
	static UQuakeSoundManager* Get(const UObject* WorldContext);

	/** Static convenience: PlaySound without writing the Get + null check. */
	static void PlaySoundEvent(const UObject* WorldContext, EQuakeSoundEvent Event, const FVector& Location);

	/**
	 * Pure helper exposed for tests. Build the row name the manager looks up
	 * for a given enum value. Stable across renames as long as the UENUM
	 * value name doesn't change.
	 */
	static FName ResolveRowName(EQuakeSoundEvent Event);

private:
	/** Pulled from UQuakeGameInstance::SoundEventTable on first use. */
	UPROPERTY()
	TObjectPtr<UDataTable> CachedTable;

	bool bTableResolved = false;

	/** Lazy resolve. Returns nullptr if no GameInstance / no table is configured. */
	UDataTable* ResolveTable();

	/** Look up a row; returns nullptr if missing. */
	const struct FQuakeSoundEvent* FindRow(EQuakeSoundEvent Event);
};
