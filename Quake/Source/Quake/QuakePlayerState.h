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

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float TimeElapsed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Powerups")
	TArray<FQuakeActivePowerup> ActivePowerups;

protected:
	virtual void Tick(float DeltaSeconds) override;

public:
	AQuakePlayerState();
};
