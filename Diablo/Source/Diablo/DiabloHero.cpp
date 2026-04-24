#include "DiabloHero.h"
#include "DiabloPlayerController.h"
#include "Diablo.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimMontage.h"

ADiabloHero::ADiabloHero()
{
	Stats.HP = 70.f;
	Stats.MaxHP = 70.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 90.f);
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeRotation(FRotator(-45.f, 225.f, 0.f));
	CameraBoom->TargetArmLength = 1800.f;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->ProjectionMode = ECameraProjectionMode::Orthographic;
	FollowCamera->OrthoWidth = 2048.f;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
}

float ADiabloHero::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDead())
	{
		return 0.f;
	}

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Stats.HP = FMath::Max(0.f, Stats.HP - ActualDamage);
	UE_LOG(LogDiablo, Display, TEXT("%s took %.0f damage (HP: %.0f/%.0f)"),
		*GetName(), ActualDamage, Stats.HP, Stats.MaxHP);

	if (Stats.HP <= 0.f)
	{
		Die();
	}

	return ActualDamage;
}

void ADiabloHero::Die()
{
	UE_LOG(LogDiablo, Display, TEXT("%s died"), *GetName());

	GetCharacterMovement()->DisableMovement();

	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}

	if (ADiabloPlayerController* PC = Cast<ADiabloPlayerController>(GetController()))
	{
		PC->OnHeroDeath();
	}
}

void ADiabloHero::Heal(float Amount)
{
	Stats.HP = FMath::Min(Stats.HP + Amount, Stats.MaxHP);
	UE_LOG(LogDiablo, Display, TEXT("%s healed %.0f HP (now %.0f/%.0f)"),
		*GetName(), Amount, Stats.HP, Stats.MaxHP);
}

void ADiabloHero::StartAttack()
{
	if (bIsAttacking || !AttackMontage || IsDead())
	{
		return;
	}

	bIsAttacking = true;
	PlayAnimMontage(AttackMontage);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ADiabloHero::OnAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
	}
}

void ADiabloHero::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
}
