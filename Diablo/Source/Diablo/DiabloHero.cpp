#include "DiabloHero.h"
#include "DiabloPlayerController.h"
#include "InventoryComponent.h"
#include "Diablo.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"

ADiabloHero::ADiabloHero()
{
	// Warrior starting stats (D1 reference)
	Stats.Str = 30.f;
	Stats.Mag = 10.f;
	Stats.Dex = 20.f;
	Stats.Vit = 25.f;
	Stats.HP = 70.f;
	Stats.MaxHP = 70.f;
	Stats.Mana = 10.f;
	Stats.MaxMana = 10.f;

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

	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}

float ADiabloHero::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDead())
	{
		return 0.f;
	}

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	const float AC = Stats.Dex / 5.f + ArmorFromEquipment;
	const float ActualDamage = FMath::Max(1.f, DamageAmount - AC);

	Stats.HP = FMath::Max(0.f, Stats.HP - ActualDamage);
	UE_LOG(LogDiablo, Display, TEXT("%s took %.0f damage (HP: %.0f/%.0f)"),
		*GetName(), ActualDamage, Stats.HP, Stats.MaxHP);

	OnStatsChanged.Broadcast();

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

	OnStatsChanged.Broadcast();
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

// ---------------------------------------------------------------------------
// XP + Leveling
// ---------------------------------------------------------------------------

const TArray<int64>& ADiabloHero::GetXPTable()
{
	// Diablo 1 XP thresholds: quartic curve approximation
	// XP needed to reach level N (cumulative). Index 0 unused, index 1 = level 1 = 0 XP.
	static TArray<int64> Table;
	if (Table.Num() == 0)
	{
		Table.SetNum(51);
		Table[0] = 0;
		Table[1] = 0;
		for (int32 L = 2; L <= 50; ++L)
		{
			const int64 L64 = static_cast<int64>(L);
			// Approximation of D1's steep quartic XP curve
			Table[L] = Table[L - 1] + 2000 * L64 * L64 * L64 / 4;
		}
	}
	return Table;
}

int64 ADiabloHero::GetXPForLevel(int32 Level) const
{
	const TArray<int64>& Table = GetXPTable();
	if (Level < 1) return 0;
	if (Level >= Table.Num()) return Table.Last();
	return Table[Level];
}

int64 ADiabloHero::GetXPForNextLevel() const
{
	return GetXPForLevel(CharLevel + 1);
}

float ADiabloHero::GetXPPercent() const
{
	if (CharLevel >= 50)
	{
		return 1.f;
	}

	const int64 CurrentLevelXP = GetXPForLevel(CharLevel);
	const int64 NextLevelXP = GetXPForLevel(CharLevel + 1);
	const int64 Range = NextLevelXP - CurrentLevelXP;
	if (Range <= 0) return 1.f;

	return FMath::Clamp(
		static_cast<float>(CurrentXP - CurrentLevelXP) / static_cast<float>(Range),
		0.f, 1.f);
}

void ADiabloHero::AwardXP(int64 Amount)
{
	if (CharLevel >= 50 || Amount <= 0)
	{
		return;
	}

	CurrentXP += Amount;
	UE_LOG(LogDiablo, Display, TEXT("%s gained %lld XP (total: %lld, need %lld for level %d)"),
		*GetName(), Amount, CurrentXP, GetXPForNextLevel(), CharLevel + 1);

	while (CharLevel < 50 && CurrentXP >= GetXPForNextLevel())
	{
		LevelUp();
	}

	OnStatsChanged.Broadcast();
}

void ADiabloHero::LevelUp()
{
	CharLevel++;
	UnspentStatPoints += 5;

	RecomputeDerivedStats();

	Stats.HP = Stats.MaxHP;
	Stats.Mana = Stats.MaxMana;

	UE_LOG(LogDiablo, Display, TEXT("%s reached level %d! (+5 stat points, HP/Mana restored)"),
		*GetName(), CharLevel);

	if (LevelUpSound)
	{
		UGameplayStatics::PlaySound2D(this, LevelUpSound);
	}
}

bool ADiabloHero::SpendStatPoint(FName StatName)
{
	if (UnspentStatPoints <= 0)
	{
		return false;
	}

	// Warrior class caps (D1 reference)
	struct FStatCap { FName Name; float* Value; float Cap; };
	FStatCap Caps[] = {
		{ FName("Str"), &Stats.Str, 250.f },
		{ FName("Mag"), &Stats.Mag, 50.f },
		{ FName("Dex"), &Stats.Dex, 60.f },
		{ FName("Vit"), &Stats.Vit, 100.f },
	};

	for (FStatCap& SC : Caps)
	{
		if (SC.Name == StatName)
		{
			if (*SC.Value >= SC.Cap)
			{
				return false;
			}

			*SC.Value += 1.f;
			UnspentStatPoints--;
			RecomputeDerivedStats();
			OnStatsChanged.Broadcast();

			UE_LOG(LogDiablo, Display, TEXT("Spent stat point: %s = %.0f (remaining: %d)"),
				*StatName.ToString(), *SC.Value, UnspentStatPoints);
			return true;
		}
	}

	return false;
}

void ADiabloHero::RecomputeDerivedStats()
{
	float BonusStr = 0.f, BonusMag = 0.f, BonusDex = 0.f, BonusVit = 0.f;
	EquipMinDamage = 0.f;
	EquipMaxDamage = 0.f;
	ArmorFromEquipment = 0.f;

	if (Inventory)
	{
		for (int32 SlotIdx = static_cast<int32>(EEquipSlot::Head);
			 SlotIdx <= static_cast<int32>(EEquipSlot::Amulet); ++SlotIdx)
		{
			const EEquipSlot Slot = static_cast<EEquipSlot>(SlotIdx);
			if (!Inventory->HasEquipped(Slot)) continue;
			const FItemInstance& Equipped = Inventory->GetEquipped(Slot);
			const UItemDefinition* Def = Equipped.Definition;
			if (!Def) continue;

			BonusStr += Def->BonusStr;
			BonusMag += Def->BonusMag;
			BonusDex += Def->BonusDex;
			BonusVit += Def->BonusVit;

			switch (Def->Category)
			{
			case EItemCategory::Weapon:
				EquipMinDamage += Def->MinDamage;
				EquipMaxDamage += Def->MaxDamage;
				break;
			case EItemCategory::Armor:
			case EItemCategory::Shield:
			case EItemCategory::Helm:
				ArmorFromEquipment += Def->ArmorClass;
				break;
			default:
				break;
			}
		}
	}

	const float TotalStr = Stats.Str + BonusStr;
	const float TotalMag = Stats.Mag + BonusMag;
	const float TotalDex = Stats.Dex + BonusDex;
	const float TotalVit = Stats.Vit + BonusVit;

	Stats.MaxHP = 70.f + (TotalVit - 25.f) * 2.f + static_cast<float>(CharLevel - 1) * 2.f;
	Stats.MaxMana = 10.f + (TotalMag - 10.f) * 1.f + static_cast<float>(CharLevel - 1) * 1.f;

	Stats.HP = FMath::Min(Stats.HP, Stats.MaxHP);
	Stats.Mana = FMath::Min(Stats.Mana, Stats.MaxMana);
}
