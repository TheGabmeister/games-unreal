#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameData.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWumpaFruitChanged, int32, NewWumpaFruit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLivesChanged, int32, NewLives);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAkuAkuCountChanged, int32, NewAkuAkuCount);

UCLASS()
class CRASHBANDICOOT_API UGameData : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
    int32 WumpaFruit = 0;
	int32 Lives = 4;
	int32 AkuAkuCount = 0;

public:
	UFUNCTION(BlueprintCallable)
	int32 GetWumpaFruit() const { return WumpaFruit; }

	UFUNCTION(BlueprintCallable)
	int32 GetLives() const { return Lives; }
	
	UFUNCTION(BlueprintCallable)
	void UpdateWumpaFruit(int32 Amount);

	UFUNCTION(BlueprintCallable)
	void UpdateLives(int32 Amount);

	UFUNCTION(BlueprintCallable)
	void UpdateAkuAkuCount(int32 Amount);

private:
	
	UPROPERTY(BlueprintAssignable)
	FOnWumpaFruitChanged OnWumpaFruitChanged;

	UPROPERTY(BlueprintAssignable)
	FOnLivesChanged OnLivesChanged;

	UPROPERTY(BlueprintAssignable)
	FOnAkuAkuCountChanged OnAkuAkuCountChanged;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
