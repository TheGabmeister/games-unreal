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

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	AQuakePlayerState();

private:
	/** World time when the level started (set in BeginPlay). */
	double LevelStartTime = 0.0;
};
