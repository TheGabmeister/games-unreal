#include "QuakeCharacter.h"
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
	SpawnAndEquipDefaultWeapon();
}

void AQuakeCharacter::SpawnAndEquipDefaultWeapon()
{
	if (!DefaultWeaponClass)
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn at origin then attach — the attach immediately overwrites the
	// transform, so the spawn location doesn't matter. Skipping the
	// FTransform overload also dodges a TSubclassOf->UClass* template
	// deduction wrinkle on UE 5.7.
	CurrentWeapon = World->SpawnActor<AQuakeWeaponBase>(DefaultWeaponClass, Params);
	if (CurrentWeapon)
	{
		CurrentWeapon->AttachToComponent(
			FirstPersonCamera,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		UE_LOG(LogQuakeCharacter, Log, TEXT("Spawned default weapon: %s"), *CurrentWeapon->GetName());
	}
	else
	{
		UE_LOG(LogQuakeCharacter, Warning, TEXT("Failed to spawn DefaultWeaponClass=%s"),
			*GetNameSafe(DefaultWeaponClass));
	}
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
		UE_LOG(LogQuakeCharacter, Log, TEXT("FireAction bound to %s"), *PC->FireAction->GetName());
	}
	else
	{
		UE_LOG(LogQuakeCharacter, Warning,
			TEXT("AQuakePlayerController::FireAction is null — assign IA_Fire in BP_QuakePlayerController defaults, "
			     "and make sure IA_Fire is mapped to LMB in IMC_Default."));
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
	UE_LOG(LogQuakeCharacter, Verbose, TEXT("OnFirePressed (CurrentWeapon=%s)"),
		CurrentWeapon ? *CurrentWeapon->GetName() : TEXT("none"));

	if (CurrentWeapon)
	{
		CurrentWeapon->TryFire(this);
	}
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
