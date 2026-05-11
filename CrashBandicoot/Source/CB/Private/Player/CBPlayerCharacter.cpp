#include "Player/CBPlayerCharacter.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "InputActionValue.h"
#include "Game/CBGameMode.h"
#include "Game/CBGameInstance.h"
#include "Player/CBCharacterMovementComponent.h"
#include "Components/BlinkComponent.h"
#include "World/AkuAkuMaskActor.h"
#include "Interfaces/InteractionInterfaces.h"
#include "Audio/CBAudioSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACBPlayerCharacter::ACBPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCBCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	// Spin attack volume — slightly larger than capsule
	SpinAttackVolume = CreateDefaultSubobject<USphereComponent>(TEXT("SpinAttackVolume"));
	SpinAttackVolume->SetupAttachment(RootComponent);
	SpinAttackVolume->SetSphereRadius(60.0f);
	SpinAttackVolume->SetCollisionProfileName(TEXT("OverlapAll"));
	SpinAttackVolume->SetGenerateOverlapEvents(true);
	SpinAttackVolume->SetActive(false);
	SpinAttackVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Blink component
	BlinkComponent = CreateDefaultSubobject<UBlinkComponent>(TEXT("BlinkComponent"));
}

void ACBPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpinCharges = MaxSpinCharges;

	SpinAttackVolume->OnComponentBeginOverlap.AddDynamic(this, &ACBPlayerCharacter::OnSpinOverlap);

	// Restore Aku Aku state from GameInstance
	if (UCBGameInstance* GI = Cast<UCBGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		EAkuAkuState Stored = static_cast<EAkuAkuState>(GI->StoredAkuAkuState);
		if (Stored != EAkuAkuState::None && Stored != EAkuAkuState::Invincible)
		{
			AkuAkuState = Stored;
			UpdateMaskVisual();
		}
	}
}

// --- Input Handlers ---

void ACBPlayerCharacter::Input_Move(const FInputActionValue& Value)
{
	FVector2D MoveInput = Value.Get<FVector2D>();
	if (MoveInput.IsNearlyZero()) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->PlayerCameraManager) return;

	const FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
	const FRotator YawRotation(0.0f, CameraRotation.Yaw, 0.0f);

	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, MoveInput.Y);
	AddMovementInput(RightDir, MoveInput.X);
}

void ACBPlayerCharacter::Input_Jump(const FInputActionValue& Value)
{
	if (bIsSpinning) StopSpin();
	Jump();
}

void ACBPlayerCharacter::Input_StopJump(const FInputActionValue& Value)
{
	StopJumping();
}

void ACBPlayerCharacter::Input_Spin(const FInputActionValue& Value)
{
	DoSpin();
}

// --- Spin System ---

void ACBPlayerCharacter::DoSpin()
{
	if (bIsSpinning || SpinCharges <= 0 || bIsDead) return;

	bIsSpinning = true;
	SpinCharges--;

	if (SpinMontage)
	{
		float PlayRate = SpinMontage->GetPlayLength() / SpinDuration;
		PlayAnimMontage(SpinMontage, PlayRate);
	}

	// Enable spin collider
	SpinAttackVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SpinAttackVolume->SetActive(true);

	// Check for actors already overlapping
	TArray<AActor*> Overlapping;
	SpinAttackVolume->GetOverlappingActors(Overlapping);
	for (AActor* Actor : Overlapping)
	{
		if (ISpinnable* Target = Cast<ISpinnable>(Actor))
		{
			Target->OnSpinHit(this);
		}
	}

	GetWorldTimerManager().SetTimer(TimerHandle_SpinDuration, this, &ACBPlayerCharacter::StopSpin, SpinDuration);

	if (!GetWorldTimerManager().IsTimerActive(TimerHandle_SpinChargeRegen))
	{
		GetWorldTimerManager().SetTimer(TimerHandle_SpinChargeRegen, this, &ACBPlayerCharacter::RegenSpinCharge, ChargeRegenInterval, true);
	}

	OnSpinStarted();
}

void ACBPlayerCharacter::StopSpin()
{
	if (!bIsSpinning) return;

	bIsSpinning = false;

	if (SpinMontage) StopAnimMontage(SpinMontage);

	// Disable spin collider
	SpinAttackVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpinAttackVolume->SetActive(false);

	GetWorldTimerManager().ClearTimer(TimerHandle_SpinDuration);

	OnSpinEnded();
}

void ACBPlayerCharacter::RegenSpinCharge()
{
	SpinCharges = FMath::Min(SpinCharges + 1, MaxSpinCharges);
	if (SpinCharges >= MaxSpinCharges)
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_SpinChargeRegen);
	}
}

void ACBPlayerCharacter::OnSpinOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsSpinning) return;

	if (ISpinnable* Target = Cast<ISpinnable>(OtherActor))
	{
		Target->OnSpinHit(this);
	}
}

// --- Jump Overrides ---

void ACBPlayerCharacter::StopJumping()
{
	if (UCBCharacterMovementComponent* CMC = GetCharacterMovement<UCBCharacterMovementComponent>())
	{
		CMC->StopJumpInput();
	}
	ResetJumpState();
}

void ACBPlayerCharacter::CheckJumpInput(float DeltaTime)
{
	JumpCurrentCountPreJump = JumpCurrentCount;

	if (UCBCharacterMovementComponent* MovementComponent = GetCharacterMovement<UCBCharacterMovementComponent>())
	{
		if (bPressedJump)
		{
			const bool bDidJump = CanJump() && MovementComponent->DoJump(bClientUpdating, DeltaTime);
			if (bDidJump)
			{
				if (!bWasJumping)
				{
					JumpCurrentCount++;
					JumpForceTimeRemaining = GetJumpMaxHoldTime();
					OnJumped();
				}
			}
			bWasJumping = bDidJump;
		}
	}
}

bool ACBPlayerCharacter::CanJumpInternal_Implementation() const
{
	if (bIsDead) return false;

	UCBCharacterMovementComponent* MovementComponent = GetCharacterMovement<UCBCharacterMovementComponent>();
	bool bJumpIsAllowed = MovementComponent->CanAttemptJump();

	if (bJumpIsAllowed)
	{
		if (!bWasJumping || GetJumpMaxHoldTime() <= 0.0f)
		{
			bJumpIsAllowed = JumpCurrentCount < JumpMaxCount;
		}
		else
		{
			const bool bJumpKeyHeld = (bPressedJump && JumpKeyHoldTime < GetJumpMaxHoldTime());
			bJumpIsAllowed = bJumpKeyHeld &&
				((JumpCurrentCount < JumpMaxCount) || (bWasJumping && JumpCurrentCount == JumpMaxCount));
		}
	}
	return bJumpIsAllowed;
}

void ACBPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Stomp detection
	if (!bIsDead && Hit.GetActor())
	{
		if (IStompable* Target = Cast<IStompable>(Hit.GetActor()))
		{
			UCBCharacterMovementComponent* CMC = GetCharacterMovement<UCBCharacterMovementComponent>();
			if (CMC && CMC->GetLastUpdateVelocity().Z < StompVelocityThreshold)
			{
				Target->OnJumpHit(this);
			}
		}
	}

	// If dead and landed on world geometry, stop movement
	if (bIsDead)
	{
		if (Hit.Component.IsValid())
		{
			ECollisionChannel Channel = Hit.Component->GetCollisionObjectType();
			if (Channel == ECC_WorldStatic || Channel == ECC_WorldDynamic)
			{
				if (UCharacterMovementComponent* MC = GetCharacterMovement())
				{
					if (MC->IsMovingOnGround())
					{
						MC->DisableMovement();
						GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					}
				}
			}
		}
	}
}

// --- Enemy Jump ---

bool ACBPlayerCharacter::IsEnemyJumpValid(UBoxComponent* HurtBox)
{
	if (bIsDead) return false;
	if (GetCharacterMovement()->Velocity.Z > 0) return false;

	const FVector& BoxExtents = HurtBox->GetScaledBoxExtent();
	const FVector& BoxCenter = HurtBox->GetComponentLocation();
	const float BoxUpperBoundZ = (BoxCenter.Z + BoxExtents.Z) - UE_KINDA_SMALL_NUMBER;

	return GetActorLocation().Z >= BoxUpperBoundZ;
}

void ACBPlayerCharacter::JumpFromEnemyHurtBox()
{
	if (UCBCharacterMovementComponent* CMC = GetCharacterMovement<UCBCharacterMovementComponent>())
	{
		CMC->DoEnemyJump();
		OnEnemyJump();
	}
}

// --- Damage System ---

void ACBPlayerCharacter::OnHit(AActor* Source)
{
	if (bIsDead) return;
	if (IsInvincible()) return;
	if (bHitInvulnerable) return;

	if (AkuAkuState == EAkuAkuState::None)
	{
		Die();
		return;
	}

	// Mask absorbs the hit
	switch (AkuAkuState)
	{
	case EAkuAkuState::TwoHits:
		AkuAkuState = EAkuAkuState::OneHit;
		break;
	case EAkuAkuState::OneHit:
		AkuAkuState = EAkuAkuState::None;
		break;
	default:
		break;
	}

	if (MaskAbsorbSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MaskAbsorbSound, GetActorLocation());
	}

	UpdateMaskVisual();
	OnAkuAkuStateChanged.Broadcast(AkuAkuState);

	// Knockback
	if (Source)
	{
		FVector KnockDir = (GetActorLocation() - Source->GetActorLocation()).GetSafeNormal2D();
		FVector Force = KnockDir * KnockbackForce * MaskAbsorbKnockbackMultiplier;
		Force.Z = KnockbackForce * 0.3f;
		LaunchCharacter(Force, true, true);
	}

	StartPostHitInvulnerability();
}

void ACBPlayerCharacter::InstantKill()
{
	Die();
}

void ACBPlayerCharacter::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	if (bIsSpinning) StopSpin();

	// Cancel invincibility
	GetWorldTimerManager().ClearTimer(TimerHandle_Invincibility);
	GetWorldTimerManager().ClearTimer(TimerHandle_PostHitInvulnerability);

	AkuAkuState = EAkuAkuState::None;
	DestroyMaskActor();

	if (BlinkComponent->IsBlinking())
	{
		BlinkComponent->StopBlinking();
	}

	// Disable input on controller
	if (APlayerController* PC = GetController<APlayerController>())
	{
		PC->DisableInput(PC);
	}

	// Zero upward velocity
	if (UCBCharacterMovementComponent* CMC = GetCharacterMovement<UCBCharacterMovementComponent>())
	{
		if (CMC->Velocity.Z > 0) CMC->Velocity.Z = 0;
	}

	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	// Notify game mode
	if (ACBGameMode* GM = GetWorld()->GetAuthGameMode<ACBGameMode>())
	{
		GM->PlayerDied();
	}

	OnCharacterDeath();
	OnPlayerDied.Broadcast();

	Destroy();
}

// --- Aku Aku System ---

void ACBPlayerCharacter::AddMask()
{
	if (MaskPickupSound)
	{
		UGameplayStatics::PlaySound2D(this, MaskPickupSound);
	}

	switch (AkuAkuState)
	{
	case EAkuAkuState::None:
		AkuAkuState = EAkuAkuState::OneHit;
		break;
	case EAkuAkuState::OneHit:
		AkuAkuState = EAkuAkuState::TwoHits;
		break;
	case EAkuAkuState::TwoHits:
		AkuAkuState = EAkuAkuState::Invincible;
		StartInvincibility();
		break;
	case EAkuAkuState::Invincible:
		return; // No effect during invincibility
	}

	UpdateMaskVisual();
	OnAkuAkuStateChanged.Broadcast(AkuAkuState);
}

void ACBPlayerCharacter::StartInvincibility()
{
	if (InvincibilitySound)
	{
		UGameplayStatics::PlaySound2D(this, InvincibilitySound);
	}

	if (UCBAudioSubsystem* Audio = GetGameInstance()->GetSubsystem<UCBAudioSubsystem>())
	{
		Audio->PlayInvincibilityMusic();
	}

	GetWorldTimerManager().SetTimer(TimerHandle_Invincibility, this, &ACBPlayerCharacter::EndInvincibility, InvincibilityDuration);
}

void ACBPlayerCharacter::EndInvincibility()
{
	AkuAkuState = EAkuAkuState::None;
	DestroyMaskActor();
	OnAkuAkuStateChanged.Broadcast(AkuAkuState);

	if (UCBAudioSubsystem* Audio = GetGameInstance()->GetSubsystem<UCBAudioSubsystem>())
	{
		Audio->PlayWorldMusic();
	}
}

void ACBPlayerCharacter::StartPostHitInvulnerability()
{
	bHitInvulnerable = true;
	BlinkComponent->StartBlinking();
	GetWorldTimerManager().SetTimer(TimerHandle_PostHitInvulnerability, this, &ACBPlayerCharacter::EndPostHitInvulnerability, PostHitInvulnerabilityDuration);
}

void ACBPlayerCharacter::EndPostHitInvulnerability()
{
	bHitInvulnerable = false;
	BlinkComponent->StopBlinking();
}

// --- Mask Visual ---

void ACBPlayerCharacter::SpawnMaskActor()
{
	if (MaskActor) return;
	if (!MaskActorClass) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	MaskActor = GetWorld()->SpawnActor<AAkuAkuMaskActor>(MaskActorClass, GetActorLocation(), FRotator::ZeroRotator, Params);
	if (MaskActor)
	{
		MaskActor->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		MaskActor->SetActorRelativeLocation(FVector(30.0f, -40.0f, 20.0f));
	}
}

void ACBPlayerCharacter::DestroyMaskActor()
{
	if (MaskActor)
	{
		MaskActor->Destroy();
		MaskActor = nullptr;
	}
}

void ACBPlayerCharacter::UpdateMaskVisual()
{
	switch (AkuAkuState)
	{
	case EAkuAkuState::None:
		DestroyMaskActor();
		break;
	case EAkuAkuState::OneHit:
		SpawnMaskActor();
		if (MaskActor)
		{
			MaskActor->SetNormalAppearance();
			MaskActor->SetActorRelativeLocation(FVector(30.0f, -40.0f, 20.0f));
		}
		break;
	case EAkuAkuState::TwoHits:
		SpawnMaskActor();
		if (MaskActor)
		{
			MaskActor->SetGoldenAppearance();
		}
		break;
	case EAkuAkuState::Invincible:
		SpawnMaskActor();
		if (MaskActor)
		{
			MaskActor->SetActorRelativeLocation(FVector(30.0f, 0.0f, 80.0f));
		}
		break;
	}
}
