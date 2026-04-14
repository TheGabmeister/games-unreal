#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "QuakeAmmoType.h"
#include "QuakeGameInstance.generated.h"

class AQuakeWeaponBase;
class UQuakeSaveGame;

/**
 * Persistent state owner that survives OpenLevel and Character respawn.
 *
 * Per SPEC section 1.4 and [CLAUDE.md](CLAUDE.md) "Architecture: State
 * Ownership", this is the home for:
 *   - Inventory: weapons owned, ammo counts, armor
 *   - Level-entry snapshot (FQuakeInventorySnapshot) restored on death
 *   - Save-game reference
 *   - Player profile, difficulty
 *
 * Phase 2: added armor fields for the damage-pipeline math.
 * Phase 4: adds ammo TMap + per-type caps (SPEC 2.1), weapon-owned slot
 *          array (SPEC 2.0 weapon-number slots 1..8), starting-loadout
 *          seeding, and the facade helpers GiveAmmo / ConsumeAmmo / GetAmmo.
 *
 * Difficulty is Phase 12; save/profile is Phase 11/13.
 *
 * Do NOT put keys, powerups, or per-level stats here — those live on
 * AQuakePlayerState (they reset on level transition; inventory does not).
 */
UCLASS()
class QUAKE_API UQuakeGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UQuakeGameInstance();

	// --- Armor (Phase 2) ---

	/**
	 * Current armor points. Replaced (not stacked) on pickup of a higher
	 * tier per SPEC section 1.2. Caps at 200 (Red Armor max).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Armor")
	float Armor = 0.f;

	/**
	 * Current armor absorption ratio. Set on pickup to 0.3 (Green),
	 * 0.6 (Yellow), or 0.8 (Red) per SPEC section 1.2. Zero means no
	 * armor — TakeDamage skips the absorption step entirely when this is
	 * zero, even if Armor somehow has a value.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Armor")
	float ArmorAbsorption = 0.f;

	// --- Weapons owned (Phase 4) ---

	/**
	 * Weapon slot array, one entry per SPEC 2.0 weapon number. Index 0 =
	 * slot 1 (Axe), index 1 = slot 2 (Shotgun), index 3 = slot 4 (Nailgun),
	 * index 6 = slot 7 (Rocket Launcher), etc. Null entries mean "not
	 * owned" and are skipped at Character spawn-time.
	 *
	 * Populated from BP_QuakeGameInstance defaults in the editor per the
	 * project's "asset slots in BP subclasses" convention. The array is
	 * pre-sized to 8 in the C++ constructor so editing the BP only requires
	 * filling the slots you want.
	 *
	 * SPEC 1.4 starting loadout is "Axe, 25 shells, no other weapons" —
	 * Phase 4 grants the Shotgun by default too (slot 2) as a development
	 * convenience until weapon pickups land in a later phase. Revert to
	 * Axe-only when AQuakePickup_Weapon exists.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Weapons")
	TArray<TSubclassOf<AQuakeWeaponBase>> OwnedWeaponClasses;

	/** Grant a weapon to the player. Stores into the slot index matching its weapon number (1-based → 0-based). */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Weapons")
	void GiveWeapon(int32 SlotNumberOneBased, TSubclassOf<AQuakeWeaponBase> WeaponClass);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Weapons")
	bool OwnsWeaponInSlot(int32 SlotIndexZeroBased) const;

	// --- Ammo (Phase 4) ---

	/**
	 * Current ammo counts keyed by type. Entries for None are ignored.
	 * Values are clamped to GetAmmoCap on every Give.
	 *
	 * Not a UPROPERTY because TMap<EQuakeAmmoType, int32> is reflection-
	 * supported BUT editing defaults from BP is clunky; we seed it in
	 * Init() from the starting loadout instead.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Ammo")
	int32 GetAmmo(EQuakeAmmoType Type) const;

	/** Add ammo, clamped to GetAmmoCap(Type). Returns the amount actually added. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Ammo")
	int32 GiveAmmo(EQuakeAmmoType Type, int32 Amount);

	/** Try to consume ammo. Returns true iff the full amount was available and deducted. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Ammo")
	bool ConsumeAmmo(EQuakeAmmoType Type, int32 Amount);

	/**
	 * Max carry for the given ammo type per SPEC section 2.1:
	 *     Shells 100, Nails 200, Rockets 100, Cells 100.
	 * Returns 0 for None so callers can loop over all types without a
	 * special case.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Ammo")
	static int32 GetAmmoCap(EQuakeAmmoType Type);

	// --- Phase 11: save/load ---

	/**
	 * Snapshot the current game state and write it to SlotName. Captures
	 * GameInstance inventory + the current level's PlayerState / per-actor
	 * state via the authoritative GameMode (DESIGN 6.2). Returns true on
	 * disk write success.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveCurrentState(const FString& SlotName);

	/**
	 * Load from SlotName: restore GameInstance fields immediately, stash
	 * the save as a pending load, then OpenLevel on the saved level name.
	 * The new GameMode's BeginPlay pulls the pending load and finishes
	 * the restore (pawn transform, PlayerState, per-actor records,
	 * consumed pickups). Returns true on slot load success.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadFromSlot(const FString& SlotName);

	/**
	 * GameMode pulls this on BeginPlay. Returns the pending load (if any)
	 * and clears it. Null when the world was opened via normal navigation
	 * (not via LoadFromSlot).
	 */
	UQuakeSaveGame* ConsumePendingLoad();

	/** Format "auto_<profile>" / "quick_<profile>". v1: profile = "default". */
	static FString BuildAutoSlotName();
	static FString BuildQuickSlotName();

	// --- Init hook ---

	virtual void Init() override;

private:
	/** Live ammo counts. Keyed on uint8(EQuakeAmmoType) so reflection is unnecessary. */
	TMap<EQuakeAmmoType, int32> AmmoCounts;

	/**
	 * Populated by LoadFromSlot before OpenLevel; consumed by the new
	 * world's AQuakeGameMode::BeginPlay. Kept as a UPROPERTY so the GC
	 * keeps the SaveGame alive across the level swap.
	 */
	UPROPERTY()
	TObjectPtr<UQuakeSaveGame> PendingLoad;

	/** Write inventory + profile fields onto a SaveGame we own. */
	void CaptureInventorySnapshot(UQuakeSaveGame& Out) const;

	/** Restore inventory + profile fields from a loaded SaveGame. */
	void ApplyInventorySnapshot(const UQuakeSaveGame& In);

	friend class AQuakeGameMode;
};
