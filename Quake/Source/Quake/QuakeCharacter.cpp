#include "QuakeCharacter.h"
#include "QuakeCollisionChannels.h"
#include "QuakeCharacterMovementComponent.h"
#include "QuakeDamageType.h"
#include "QuakeGameInstance.h"
#include "QuakePlayerController.h"
#include "QuakeWeaponBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeCharacter, Log, All);

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PostProcessComponent.h"
#include "EnhancedInputComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"

AQuakeCharacter::AQuakeCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UQuakeCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	// SPEC section 1.6: player capsule radius 35, half-height 90.
	GetCapsuleComponent()->InitCapsuleSize(35.f, 90.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(QuakeCollision::ECC_Pickup, ECR_Overlap);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	DamageFlashPostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("DamageFlashPostProcess"));
	DamageFlashPostProcess->SetupAttachment(FirstPersonCamera);
	DamageFlashPostProcess->bUnbound = true;  // Apply regardless of camera location.

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// All movement parameters are set in UQuakeCharacterMovementComponent's
	// constructor — intentionally not duplicated here.
}

void AQuakeCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	SpawnOwnedWeapons();
}

void AQuakeCharacter::SpawnOwnedWeapons()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	UQuakeGameInstance* GameInstance = World->GetGameInstance<UQuakeGameInstance>();
	if (!GameInstance)
	{
		UE_LOG(LogQuakeCharacter, Warning,
			TEXT("SpawnOwnedWeapons: no UQuakeGameInstance — set Game Instance Class in Project Settings"));
		return;
	}

	// Mirror the GameInstance's 8-slot array so index i always maps to
	// SPEC 2.0 weapon number i+1, regardless of how many slots are
	// actually filled.
	WeaponInstances.SetNum(8);

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int32 FirstOwnedSlot = -1;
	for (int32 Slot = 0; Slot < 8 && Slot < GameInstance->OwnedWeaponClasses.Num(); ++Slot)
	{
		TSubclassOf<AQuakeWeaponBase> Class = GameInstance->OwnedWeaponClasses[Slot];
		if (!Class)
		{
			continue;
		}

		// Spawn at origin then attach — the attach immediately overwrites
		// the transform, so the spawn location doesn't matter. Skipping
		// the FTransform overload also dodges a TSubclassOf->UClass*
		// template deduction wrinkle on UE 5.7.
		AQuakeWeaponBase* Weapon = World->SpawnActor<AQuakeWeaponBase>(Class, Params);
		if (!Weapon)
		{
			UE_LOG(LogQuakeCharacter, Warning, TEXT("Failed to spawn weapon in slot %d (%s)"),
				Slot + 1, *GetNameSafe(Class));
			continue;
		}
		Weapon->AttachToComponent(
			FirstPersonCamera,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Weapon->SetActorHiddenInGame(true);  // Hidden until equipped.
		WeaponInstances[Slot] = Weapon;

		if (FirstOwnedSlot == -1)
		{
			FirstOwnedSlot = Slot;
		}
	}

	if (FirstOwnedSlot != -1)
	{
		SwitchToWeaponSlot(FirstOwnedSlot);
	}
	else
	{
		UE_LOG(LogQuakeCharacter, Warning,
			TEXT("SpawnOwnedWeapons: no weapons in UQuakeGameInstance::OwnedWeaponClasses — "
			     "populate BP_QuakeGameInstance in the editor"));
	}
}

bool AQuakeCharacter::SwitchToWeaponSlot(int32 SlotIndexZeroBased)
{
	if (SlotIndexZeroBased < 0 || SlotIndexZeroBased >= WeaponInstances.Num())
	{
		return false;
	}
	AQuakeWeaponBase* Target = WeaponInstances[SlotIndexZeroBased];
	if (!Target || Target == CurrentWeapon)
	{
		return false;
	}

	// Hide the outgoing viewmodel so only the active weapon is visible.
	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorHiddenInGame(true);
	}
	CurrentWeapon = Target;
	CurrentWeaponSlot = SlotIndexZeroBased;
	CurrentWeapon->SetActorHiddenInGame(false);
	return true;
}

void AQuakeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	AQuakePlayerController* PC = GetController<AQuakePlayerController>();
	if (!PC) return;

	UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInput->BindAction(PC->MoveAction, ETriggerEvent::Triggered, this, &AQuakeCharacter::Move);
	EnhancedInput->BindAction(PC->LookAction, ETriggerEvent::Triggered, this, &AQuakeCharacter::Look);
	EnhancedInput->BindAction(PC->JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
	EnhancedInput->BindAction(PC->JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

	if (PC->FireAction)
	{
		EnhancedInput->BindAction(PC->FireAction, ETriggerEvent::Triggered, this, &AQuakeCharacter::OnFirePressed);
	}
	else
	{
		UE_LOG(LogQuakeCharacter, Warning,
			TEXT("AQuakePlayerController::FireAction is null — assign IA_Fire in BP_QuakePlayerController defaults, "
			     "and make sure IA_Fire is mapped to LMB in IMC_Default."));
	}

	// Weapon swap. Per SPEC 2.2 "instant via number keys (1-8)". IA_Weapon1 /
	// IA_Weapon2 / IA_Weapon4 / IA_Weapon7 slots on BP_QuakePlayerController
	// are authored in the editor and mapped to keyboard 1 / 2 / 4 / 7 in
	// IMC_Default. These are the four v1 weapons (Axe, Shotgun, Nailgun,
	// Rocket Launcher) — later phases will add slots for SSG / GL / SNG /
	// Thunderbolt.
	if (PC->Weapon1Action)
	{
		EnhancedInput->BindAction(PC->Weapon1Action, ETriggerEvent::Started, this, &AQuakeCharacter::OnWeapon1Pressed);
	}
	if (PC->Weapon2Action)
	{
		EnhancedInput->BindAction(PC->Weapon2Action, ETriggerEvent::Started, this, &AQuakeCharacter::OnWeapon2Pressed);
	}
	if (PC->Weapon4Action)
	{
		EnhancedInput->BindAction(PC->Weapon4Action, ETriggerEvent::Started, this, &AQuakeCharacter::OnWeapon4Pressed);
	}
	if (PC->Weapon7Action)
	{
		EnhancedInput->BindAction(PC->Weapon7Action, ETriggerEvent::Started, this, &AQuakeCharacter::OnWeapon7Pressed);
	}
}

void AQuakeCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, Input.Y);
	AddMovementInput(RightDir, Input.X);
}

void AQuakeCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();

	AddControllerYawInput(Input.X * LookSensitivity);
	AddControllerPitchInput(-Input.Y * LookSensitivity);
}

void AQuakeCharacter::OnFirePressed(const FInputActionValue& /*Value*/)
{
	// Bound to ETriggerEvent::Triggered so a "Hold" trigger on the IA can
	// auto-repeat. Cooldown enforcement lives in AQuakeWeaponBase::TryFire,
	// which gates the per-tick spam to the weapon's RoF.
	if (CurrentWeapon)
	{
		CurrentWeapon->TryFire(this);
	}
}

void AQuakeCharacter::OnWeapon1Pressed(const FInputActionValue& /*Value*/)
{
	SwitchToWeaponSlot(0);
}

void AQuakeCharacter::OnWeapon2Pressed(const FInputActionValue& /*Value*/)
{
	SwitchToWeaponSlot(1);
}

void AQuakeCharacter::OnWeapon4Pressed(const FInputActionValue& /*Value*/)
{
	// Slot index 3 = SPEC 2.0 weapon number 4 = Nailgun.
	SwitchToWeaponSlot(3);
}

void AQuakeCharacter::OnWeapon7Pressed(const FInputActionValue& /*Value*/)
{
	// Slot index 6 = SPEC 2.0 weapon number 7 = Rocket Launcher.
	SwitchToWeaponSlot(6);
}

int32 AQuakeCharacter::GiveAmmo(EQuakeAmmoType Type, int32 Amount)
{
	UQuakeGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance<UQuakeGameInstance>() : nullptr;
	return GameInstance ? GameInstance->GiveAmmo(Type, Amount) : 0;
}

bool AQuakeCharacter::ConsumeAmmo(EQuakeAmmoType Type, int32 Amount)
{
	UQuakeGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance<UQuakeGameInstance>() : nullptr;
	return GameInstance ? GameInstance->ConsumeAmmo(Type, Amount) : false;
}

int32 AQuakeCharacter::GetAmmo(EQuakeAmmoType Type) const
{
	UQuakeGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance<UQuakeGameInstance>() : nullptr;
	return GameInstance ? GameInstance->GetAmmo(Type) : 0;
}

void AQuakeCharacter::GiveHealth(float Amount, bool bOvercharge)
{
	if (IsDead() || Amount <= 0.f)
	{
		return;
	}
	const float Cap = bOvercharge ? GetOverchargeCap() : MaxHealth;
	Health = FMath::Min(Health + Amount, Cap);
}

int32 AQuakeCharacter::PickAutoSwitchWeaponSlot(
	const TArray<bool>& SlotOwnedMask,
	const TArray<bool>& SlotHasAmmoMask,
	int32 ExcludeSlot)
{
	// SPEC 2.2 empty-ammo priority: RL -> SNG -> SSG -> NG -> SG -> Axe.
	// Thunderbolt (slot 8, index 7) and GL (slot 6, index 5) are
	// deliberately NOT in the list — they are "kept manual to avoid
	// accidental switching" per SPEC 2.2. Slot indices here are SPEC 2.0
	// weapon numbers minus one.
	static const int32 kPriorityOrder[] = { 6, 4, 2, 3, 1, 0 };

	for (const int32 Slot : kPriorityOrder)
	{
		if (Slot == ExcludeSlot)
		{
			continue;
		}
		if (!SlotOwnedMask.IsValidIndex(Slot) || !SlotOwnedMask[Slot])
		{
			continue;
		}
		if (!SlotHasAmmoMask.IsValidIndex(Slot) || !SlotHasAmmoMask[Slot])
		{
			continue;
		}
		return Slot;
	}
	return -1;
}

bool AQuakeCharacter::AutoSwitchFromEmptyWeapon()
{
	UWorld* World = GetWorld();
	UQuakeGameInstance* GameInstance = World ? World->GetGameInstance<UQuakeGameInstance>() : nullptr;
	if (!GameInstance)
	{
		return false;
	}

	// Build the ownership + ammo masks for PickAutoSwitchWeaponSlot. The
	// Axe (AmmoType::None) is always "has ammo" — it's the terminal
	// fallback that guarantees we never auto-switch to an empty weapon
	// when the player at least owns their starting axe.
	TArray<bool> OwnedMask;
	TArray<bool> HasAmmoMask;
	OwnedMask.Init(false, 8);
	HasAmmoMask.Init(false, 8);

	const int32 NumInstances = FMath::Min(WeaponInstances.Num(), 8);
	for (int32 Slot = 0; Slot < NumInstances; ++Slot)
	{
		const AQuakeWeaponBase* Weapon = WeaponInstances[Slot];
		if (!Weapon)
		{
			continue;
		}
		OwnedMask[Slot] = true;

		if (Weapon->AmmoType == EQuakeAmmoType::None)
		{
			// Axe: infinite ammo gate.
			HasAmmoMask[Slot] = true;
		}
		else
		{
			HasAmmoMask[Slot] = GameInstance->GetAmmo(Weapon->AmmoType) >= Weapon->AmmoPerShot;
		}
	}

	const int32 BestSlot = PickAutoSwitchWeaponSlot(OwnedMask, HasAmmoMask, CurrentWeaponSlot);
	if (BestSlot < 0)
	{
		return false;
	}
	return SwitchToWeaponSlot(BestSlot);
}

void AQuakeCharacter::ApplyArmorAbsorption(
	float InHealth, float InArmor, float InAbsorption, float InDamage,
	float& OutHealth, float& OutArmor)
{
	// Quake formula (qwsv-2.40 source/world.c T_Damage):
	//     save = ceil(armortype * damage)
	//     if (save >= armorvalue) save = armorvalue
	//     take = damage - save
	// Where armortype is the absorption ratio (0.3/0.6/0.8 for Green/Yellow/Red)
	// and "save" is the portion taken by armor instead of HP. The remainder
	// (take) hits HP. If absorption is zero or armor is empty, save is zero
	// and the formula reduces to OutHealth = InHealth - InDamage.
	//
	// Float-imprecision guard: 0.3f is not exactly representable in IEEE 754,
	// so 0.3f * 50.0f computes to 15.000000596..., which a naive CeilToFloat
	// rounds to 16 instead of 15. Subtract UE_KINDA_SMALL_NUMBER (1e-4) before
	// the ceil so values that are "essentially integer" don't get bumped up by
	// one. The epsilon is well below any meaningful damage fraction
	// (1e-4 << 1) so genuine fractional results (e.g. ceil(7.5) = 8) are
	// unaffected.
	const float Product = InAbsorption * InDamage;
	const float Save = FMath::Min(FMath::CeilToFloat(Product - UE_KINDA_SMALL_NUMBER), InArmor);
	const float Take = InDamage - Save;

	OutArmor = InArmor - Save;
	OutHealth = InHealth - Take;
}

float AQuakeCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.f || IsDead())
	{
		return 0.f;
	}

	// Resolve damage-type metadata via the shared-base CDO cast pattern from
	// SPEC section 1.5. Never branch on leaf class identity — read fields
	// uniformly off the CDO. The fall-back to UQuakeDamageType protects
	// against callers that omit a damage type entirely; in that case all
	// flags read as the abstract base defaults.
	const UQuakeDamageType* DT = Cast<UQuakeDamageType>(
		DamageEvent.DamageTypeClass
			? DamageEvent.DamageTypeClass->GetDefaultObject()
			: UQuakeDamageType::StaticClass()->GetDefaultObject());

	// Self-damage scaling: when the instigator pawn is us and the damage
	// type allows self-damage (bSelfDamage), apply SelfDamageScale (0.5 for
	// explosives = rocket-jump survivable, 1.0 default).
	float ScaledDamage = ActualDamage;
	if (DT)
	{
		const APawn* InstigatorPawn = EventInstigator ? EventInstigator->GetPawn() : nullptr;
		if (InstigatorPawn == this)
		{
			if (!DT->bSelfDamage)
			{
				return 0.f;
			}
			ScaledDamage *= DT->SelfDamageScale;
		}
	}

	// Armor absorption — only if the damage type does not bypass armor.
	UQuakeGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance<UQuakeGameInstance>() : nullptr;
	float NewHealth = Health;
	float NewArmor = GameInstance ? GameInstance->Armor : 0.f;
	const float Absorption = (DT && !DT->bIgnoresArmor && GameInstance) ? GameInstance->ArmorAbsorption : 0.f;

	ApplyArmorAbsorption(Health, NewArmor, Absorption, ScaledDamage, NewHealth, NewArmor);

	Health = NewHealth;
	if (GameInstance)
	{
		GameInstance->Armor = NewArmor;
	}

	// Knockback. Quake's knockback magnitude scales with damage AND with the
	// damage type's KnockbackScale (4.0 for explosives = rocket jumps, 1.0
	// default), which is why we don't route through stock
	// ACharacter::ApplyDamageMomentum — that uses a fixed DamageImpulse and
	// would lose the damage scaling. We still reuse the engine's
	// FDamageEvent::GetBestHitInfo helper for the impulse direction so we
	// don't have to branch on FPointDamageEvent vs FRadialDamageEvent
	// ourselves.
	if (DT && DT->KnockbackScale > 0.f && GetCharacterMovement())
	{
		APawn* InstigatorPawnForHit = EventInstigator ? EventInstigator->GetPawn() : nullptr;
		FHitResult HitInfo;
		FVector ImpulseDir;
		DamageEvent.GetBestHitInfo(this, InstigatorPawnForHit, HitInfo, ImpulseDir);

		if (!ImpulseDir.IsNearlyZero())
		{
			const float ImpulseMagnitude = ScaledDamage * 30.f * DT->KnockbackScale;
			GetCharacterMovement()->AddImpulse(ImpulseDir * ImpulseMagnitude, /*bVelocityChange*/ true);
		}
	}

	// Pain feedback (visual flash). Suppressed when the damage type silences
	// the pain reaction (Lava ticks, Telefrag).
	if (!DT || !DT->bSuppressesPain)
	{
		TriggerDamageFlash(FMath::Clamp(ScaledDamage / 50.f, 0.f, 1.f));
	}

	if (Health <= 0.f)
	{
		// Phase 2 stub: just clamp and stop. Death restart flow is built
		// out in Phase 6 per SPEC section 6.4.
		Health = 0.f;
	}

	return ScaledDamage;
}

void AQuakeCharacter::TriggerDamageFlash(float /*Intensity*/)
{
	// Phase 2 stub — see header for the wiring plan when the post-process
	// material asset arrives.
}
