#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "QuakeSoundEvent.generated.h"

class USoundBase;

/**
 * Phase 14 / DESIGN 8.2 sound event catalog. Every gameplay event that
 * the audio system might play routes through one of these enum values via
 * `UQuakeSoundManager::PlaySound`. Authored asset assignment lives in
 * `DT_SoundEvents`; rows with `Sound = nullptr` make `PlaySound` a no-op so
 * gameplay ships before any audio assets exist.
 *
 * Adding a new event = add an enum value here, log site + `PlaySound` call
 * at the gameplay site, then optionally a row in `DT_SoundEvents`. Code
 * never branches on the enum — the manager looks the row up in the table.
 */
UENUM(BlueprintType)
enum class EQuakeSoundEvent : uint8
{
	None             UMETA(DisplayName = "None"),

	// --- Player ---
	PlayerFootstep   UMETA(DisplayName = "Player Footstep"),
	PlayerJump       UMETA(DisplayName = "Player Jump"),
	PlayerLand       UMETA(DisplayName = "Player Land"),
	PlayerPain       UMETA(DisplayName = "Player Pain"),
	PlayerDeath      UMETA(DisplayName = "Player Death"),
	PickupItem       UMETA(DisplayName = "Pickup (Generic)"),
	PickupWeapon     UMETA(DisplayName = "Pickup Weapon"),
	PickupPowerup    UMETA(DisplayName = "Pickup Powerup"),

	// --- Weapons ---
	WeaponAxeSwing       UMETA(DisplayName = "Axe Swing"),
	WeaponShotgunFire    UMETA(DisplayName = "Shotgun Fire"),
	WeaponNailgunFire    UMETA(DisplayName = "Nailgun Fire"),
	WeaponRocketFire     UMETA(DisplayName = "Rocket Launcher Fire"),
	WeaponEmptyClick     UMETA(DisplayName = "Weapon Empty Click"),
	WeaponThunderboltHum UMETA(DisplayName = "Thunderbolt Hum"),
	GrenadeBounce        UMETA(DisplayName = "Grenade Bounce"),
	RocketExplode        UMETA(DisplayName = "Rocket Explode"),

	// --- Enemies ---
	EnemyAlert       UMETA(DisplayName = "Enemy Alert"),
	EnemyPain        UMETA(DisplayName = "Enemy Pain"),
	EnemyDeath       UMETA(DisplayName = "Enemy Death"),
	EnemyAttack      UMETA(DisplayName = "Enemy Attack"),
	EnemyIdle        UMETA(DisplayName = "Enemy Idle"),

	// --- World ---
	DoorOpen         UMETA(DisplayName = "Door Open"),
	DoorClose        UMETA(DisplayName = "Door Close"),
	ButtonPress      UMETA(DisplayName = "Button Press"),
	Teleport         UMETA(DisplayName = "Teleport"),
	SecretFound      UMETA(DisplayName = "Secret Found"),
};

/**
 * Music tracks. PlayMusic / StopMusic on the sound manager. Stub for v1 —
 * per-level ambient is the only entry. Per DESIGN 8.2.
 */
UENUM(BlueprintType)
enum class EQuakeMusicTrack : uint8
{
	None         UMETA(DisplayName = "None"),
	LevelAmbient UMETA(DisplayName = "Level Ambient"),
};

/**
 * One row in DT_SoundEvents. Keyed by FName("EQuakeSoundEvent::PlayerJump")
 * shape — the sound manager builds the lookup name from the enum value via
 * `UEnum::GetNameStringByValue`. `Sound = nullptr` means "no asset assigned
 * yet"; the manager logs at Verbose and returns. Volume / pitch defaults
 * live here so designers can tweak per-row without touching code.
 */
USTRUCT(BlueprintType)
struct FQuakeSoundEvent : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (ClampMin = "0.0"))
	float VolumeMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (ClampMin = "0.01"))
	float PitchMultiplier = 1.f;
};
