#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "QuakeKeyColor.h"
#include "QuakePowerup.h"
#include "QuakePlayerState.generated.h"

class UQuakeSaveGame;

USTRUCT(BlueprintType)
struct FQuakeActivePowerup
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EQuakePowerup Type = EQuakePowerup::None;

	UPROPERTY(BlueprintReadOnly)
	float RemainingTime = 0.f;
};

UCLASS()
class QUAKE_API AQuakePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 Secrets = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 Deaths = 0;

	/** Computed from world time — no tick accumulation needed. */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetTimeElapsed() const;

	UPROPERTY(BlueprintReadOnly, Category = "Powerups")
	TArray<FQuakeActivePowerup> ActivePowerups;

	/** Call after adding a powerup to start the expiry tick. */
	void EnablePowerupTick();

	/**
	 * SPEC 4.3 grant logic: if the same type is already active, refresh
	 * additively capped at 60 s. Otherwise push a new entry and arm tick.
	 * Quad pickups pass Duration = 30 per SPEC 4.3.
	 */
	UFUNCTION(BlueprintCallable, Category = "Powerups")
	void GivePowerup(EQuakePowerup Type, float Duration);

	/** True iff the given powerup has a live entry. */
	UFUNCTION(BlueprintCallable, Category = "Powerups")
	bool HasPowerup(EQuakePowerup Type) const;

	/** Remaining seconds for the given powerup, 0 if not active. */
	UFUNCTION(BlueprintCallable, Category = "Powerups")
	float GetPowerupRemaining(EQuakePowerup Type) const;

	/** SPEC 4.4: keys reset on level transition / death-restart, not consumed on use. */
	UFUNCTION(BlueprintCallable, Category = "Keys")
	bool HasKey(EQuakeKeyColor Color) const;

	/** Add a key. No-op if already held (SPEC 4.4: picking up a held key is silent). */
	UFUNCTION(BlueprintCallable, Category = "Keys")
	void GiveKey(EQuakeKeyColor Color);

	/**
	 * SPEC section 1.4 / 6.4: empty the per-life state (ActivePowerups and
	 * Keys). Called from the death-restart flow because UE's PlayerState
	 * persists across pawn respawn — powerups and keys would otherwise
	 * survive death.
	 *
	 * Does NOT reset Kills / Secrets / TimeElapsed / Deaths: those persist
	 * across the level attempt and only clear on a true OpenLevel.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lifecycle")
	void ClearPerLifeState();

	/** SPEC 5.9: +1 when the player kills a marked-kill-target enemy. */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddKillCredit() { ++Kills; }

	/** SPEC 5.9: +1 on first entry into a AQuakeTrigger_Secret. */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddSecretCredit() { ++Secrets; }

	/** SPEC 6.4: incremented once per death, before the snapshot restore. */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddDeath() { ++Deaths; }

	AQuakePlayerState();

	virtual void Tick(float DeltaSeconds) override;

	/** SPEC 4.3: same-powerup refresh cap. */
	static constexpr float GetPowerupMaxDuration() { return 60.f; }

	// --- Phase 11: save/load ---

	/** Write Kills/Secrets/Deaths/ElapsedAtSave/ActivePowerups/Keys into the save. */
	void CaptureToSave(UQuakeSaveGame& Out) const;

	/**
	 * Restore PlayerState fields. Translates the saved ElapsedAtSave into
	 * LevelStartTime = WorldTimeNow - ElapsedAtSave so GetTimeElapsed()
	 * resumes at the saved value. Re-arms powerup tick if any entries are
	 * live.
	 */
	void ApplyFromSave(const UQuakeSaveGame& In, double WorldTimeNow);

protected:
	virtual void BeginPlay() override;

private:
	/** World time when the level started (set in BeginPlay). */
	double LevelStartTime = 0.0;

	/**
	 * Keys currently held this level. Cleared by ClearPerLifeState. TArray
	 * rather than TSet because Phase 11's save-game path needs a
	 * reflection-friendly container and TSet of BP enums is wobbly on that
	 * front; the v1 key alphabet is only two entries so linear Contains is
	 * fine.
	 */
	UPROPERTY()
	TArray<EQuakeKeyColor> Keys;
};
