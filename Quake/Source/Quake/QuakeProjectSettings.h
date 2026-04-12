#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "QuakeProjectSettings.generated.h"

class UDataTable;

/**
 * Project-wide configuration for the Quake module. Shows up in
 * Project Settings > Game > Quake. Holds asset/table references that
 * subsystems and gameplay code need — no Blueprint subclass required.
 *
 * Read anywhere via GetDefault<UQuakeProjectSettings>().
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Quake"))
class QUAKE_API UQuakeProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetContainerName() const override { return FName("Project"); }
	virtual FName GetCategoryName() const override { return FName("Game"); }

	/** Sound event lookup table. Read by the sound manager subsystem (Phase 14). */
	UPROPERTY(EditAnywhere, Config, Category = "Audio")
	TSoftObjectPtr<UDataTable> SoundEventTable;

	/** Per-enemy-type balance stats (FQuakeEnemyStatsRow). */
	UPROPERTY(EditAnywhere, Config, Category = "Balance")
	TSoftObjectPtr<UDataTable> EnemyStatsTable;

	/** Per-weapon balance stats (FQuakeWeaponStatsRow). */
	UPROPERTY(EditAnywhere, Config, Category = "Balance")
	TSoftObjectPtr<UDataTable> WeaponStatsTable;
};
