#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "QuakePlayerState.generated.h"

USTRUCT(BlueprintType)
struct FQuakeActivePowerup
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName PowerupType;

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
	 * SPEC section 1.4 / 6.4: empty the per-life state (ActivePowerups, and
	 * Keys once they land in Phase 10). Called from the death-restart flow
	 * because UE's PlayerState persists across pawn respawn — powerups
	 * would otherwise survive death.
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

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	AQuakePlayerState();

private:
	/** World time when the level started (set in BeginPlay). */
	double LevelStartTime = 0.0;
};
