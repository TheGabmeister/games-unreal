#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "QuakeCharacter.generated.h"

class UCameraComponent;
class UPostProcessComponent;
class AQuakeWeaponBase;

UCLASS()
class QUAKE_API AQuakeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AQuakeCharacter(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UPostProcessComponent> DamageFlashPostProcess;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float LookSensitivity = 0.5f;

	// --- Phase 2: health + damage pipeline ---

	/** Read-only accessor for the HUD overlay. SPEC: only TakeDamage mutates Health. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead() const { return Health <= 0.f; }

	// --- Phase 2: weapon ---

	/**
	 * Default weapon class spawned in BeginPlay. SPEC section 2: the player
	 * always carries the Axe. Filled in BP defaults; the C++ class itself
	 * leaves it null so unit tests can construct the character without
	 * dragging the weapon header in.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons")
	TSubclassOf<AQuakeWeaponBase> DefaultWeaponClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapons")
	TObjectPtr<AQuakeWeaponBase> CurrentWeapon;

	/**
	 * Damage absorption math, extracted as a pure static helper so it can
	 * be unit-tested without spinning up a world or a character. Implements
	 * the original Quake formula:
	 *     save = ceil(absorption * damage)   <-- absorbed by armor
	 *     if (save > armor) save = armor     <-- can't drain more than we have
	 *     take = damage - save               <-- spills to HP
	 *
	 * For 100 HP + 100 green armor (absorption=0.3) hit by 50 damage:
	 *     save = 15, take = 35  -> HP=65, Armor=85.
	 *
	 * Pass InAbsorption = 0.0 (or InArmor = 0) to bypass armor entirely:
	 * the function reduces to OutHealth = InHealth - InDamage.
	 */
	static void ApplyArmorAbsorption(
		float InHealth, float InArmor, float InAbsorption, float InDamage,
		float& OutHealth, float& OutArmor);

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Maximum HP. SPEC section 1.1 player stats: Health (max) = 100. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	/**
	 * Live HP. Decremented exclusively by TakeDamage per SPEC section 1.5
	 * "no code outside TakeDamage mutates health directly". Protected so
	 * external readers go through GetHealth().
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Health")
	float Health = 100.f;

	/**
	 * Trigger the screen-flash damage feedback. Phase 2 stub: writes the
	 * intensity into a dynamic material instance parameter on the post-
	 * process component if one exists, otherwise no-ops. The actual
	 * post-process material asset is wired up in a later phase per
	 * SPEC section 7.1; this hook exists now so the call site in
	 * TakeDamage doesn't have to grow later.
	 */
	void TriggerDamageFlash(float Intensity);

private:
	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
	void OnFirePressed(const struct FInputActionValue& Value);

	/** Equipped to CurrentWeapon in BeginPlay if DefaultWeaponClass is set. */
	void SpawnAndEquipDefaultWeapon();
};
