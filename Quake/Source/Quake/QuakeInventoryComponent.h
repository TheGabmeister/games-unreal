#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuakeAmmoType.h"
#include "QuakeInventorySnapshot.h"
#include "QuakeInventoryComponent.generated.h"

class AQuakeWeaponBase;

/**
 * Per-pawn inventory: ammo counts, armor, and owned weapon classes.
 *
 * Lives on AQuakeCharacter as a CreateDefaultSubobject component. Replaces
 * the previous "all inventory lives on UQuakeGameInstance" design — a
 * single-player shortcut with no MP analog, since UGameInstance is
 * per-local-machine, not per-player. Component-on-pawn matches the
 * Lyra/UT convention and makes MP replication a drop-in follow-up
 * (just add UPROPERTY(Replicated) + GetLifetimeReplicatedProps).
 *
 * Cross-level persistence happens via a passive snapshot struct held on
 * UQuakeGameInstance:
 *   - Level transition: AQuakeTrigger_Exit serializes the live component
 *     into GI->TransitSnapshot before calling OpenLevel.
 *   - OpenLevel destroys the old pawn.
 *   - New pawn's component hydrates from GI->TransitSnapshot in
 *     InitializeComponent (before any BeginPlay fires, so GameMode's
 *     level-entry auto-save + SnapshotForLevelEntry see the hydrated state).
 *   - TransitSnapshot.bValid is cleared after consumption.
 *
 * Death restart uses the same mechanism: RequestRestartFromDeath populates
 * TransitSnapshot from LevelEntrySnapshot, then respawns; the new pawn's
 * component hydrates identically.
 *
 * Tests construct this via NewObject<UQuakeInventoryComponent>() with no
 * world — same standalone-testable shape the old UQuakeGameInstance had
 * for the inventory API.
 */
UCLASS(ClassGroup = (Quake), meta = (BlueprintSpawnableComponent))
class QUAKE_API UQuakeInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuakeInventoryComponent();

	// --- Armor (Phase 2) ---

	UFUNCTION(BlueprintCallable, Category = "Inventory|Armor")
	float GetArmor() const { return Armor; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Armor")
	float GetArmorAbsorption() const { return ArmorAbsorption; }

	/** Tier-replacement write (called from AQuakePickup_Armor). */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Armor")
	void SetArmor(float NewArmor, float NewAbsorption);

	/** Post-absorption damage application. Called from AQuakeCharacter::TakeDamage. */
	void ApplyArmorDamage(float NewArmor);

	// --- Weapons ---

	/**
	 * Owned weapon classes, one entry per SPEC 2.0 slot. Index 0 = slot 1 (Axe),
	 * etc. Null entries = "not owned". Pre-sized to NumWeaponSlots in the
	 * constructor. EditDefaultsOnly so BP_QuakeCharacter fills the starting
	 * loadout per-instance.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Weapons")
	TArray<TSubclassOf<AQuakeWeaponBase>> OwnedWeaponClasses;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Weapons")
	void GiveWeapon(int32 SlotNumberOneBased, TSubclassOf<AQuakeWeaponBase> WeaponClass);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Weapons")
	bool OwnsWeaponInSlot(int32 SlotIndexZeroBased) const;

	// --- Ammo ---

	UFUNCTION(BlueprintCallable, Category = "Inventory|Ammo")
	int32 GetAmmo(EQuakeAmmoType Type) const;

	/** Add ammo, clamped to GetAmmoCap. Returns the amount actually added. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Ammo")
	int32 GiveAmmo(EQuakeAmmoType Type, int32 Amount);

	/** Try to consume ammo. Returns true iff the full amount was available. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Ammo")
	bool ConsumeAmmo(EQuakeAmmoType Type, int32 Amount);

	/** SPEC 2.1 per-type caps. Pure (no state). */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Ammo")
	static int32 GetAmmoCap(EQuakeAmmoType Type);

	/**
	 * Starting shells granted on first-run hydration (no transit snapshot).
	 * Exposed on BP_QuakeCharacter so designers can tune without a recompile.
	 * Other ammo types default to 0.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Ammo",
		meta = (ClampMin = "0"))
	int32 StartingShells = 25;

	// --- Snapshot round-trip ---

	/** Copy state into Out. Used by save-game + cross-level transit + level-entry capture. */
	void SerializeTo(FQuakeInventorySnapshot& Out) const;

	/**
	 * Replace state from In. Also pre-seeds missing ammo types so downstream
	 * GetAmmo always finds a key (matches original RestoreFromLevelEntrySnapshot
	 * behavior).
	 */
	void DeserializeFrom(const FQuakeInventorySnapshot& In);

protected:
	virtual void InitializeComponent() override;

private:
	/** Current armor points. Replaced (not stacked) on pickup. Caps at 200 (Red). */
	UPROPERTY(VisibleInstanceOnly, Category = "Inventory|Armor", meta = (AllowPrivateAccess = "true"))
	float Armor = 0.f;

	/** Absorption ratio (0.3 Green / 0.6 Yellow / 0.8 Red). 0 = no armor. */
	UPROPERTY(VisibleInstanceOnly, Category = "Inventory|Armor", meta = (AllowPrivateAccess = "true"))
	float ArmorAbsorption = 0.f;

	/**
	 * Live ammo counts. Not a UPROPERTY — TMap<UENUM, int32> is reflection-
	 * supported but BP-editing defaults is clunky; seed via SeedDefaultsIfEmpty
	 * on first-run hydration instead.
	 */
	TMap<EQuakeAmmoType, int32> AmmoCounts;

	/** First-run ammo seeding when no transit snapshot is valid. */
	void SeedDefaultsIfEmpty();
};
