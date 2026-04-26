#include "DiabloEnemy.h"
#include "DiabloHero.h"
#include "DiabloAIController.h"
#include "DroppedItem.h"
#include "Firebolt.h"
#include "ItemDefinition.h"
#include "AffixGenerator.h"
#include "SpellProjectile.h"
#include "Diablo.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"

ADiabloEnemy::ADiabloEnemy()
{
	Stats.HP = 30.f;
	Stats.MaxHP = 30.f;
	XPReward = 2000;
	ProjectileClass = AFirebolt::StaticClass();

	GetCapsuleComponent()->InitCapsuleSize(34.f, 90.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);

	AIControllerClass = ADiabloAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	ApplyArchetypeDefaults();
}

void ADiabloEnemy::BeginPlay()
{
	Super::BeginPlay();
	ApplyArchetypeDefaults();
}

float ADiabloEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
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
		UE_LOG(LogDiablo, Display, TEXT("%s died"), *GetName());

		if (!bIsSummonedMinion)
		{
			if (ADiabloHero* Hero = Cast<ADiabloHero>(UGameplayStatics::GetPlayerPawn(this, 0)))
			{
				const int32 LevelDiff = Hero->CharLevel - MonsterLevel;
				if (LevelDiff < 10)
				{
					Hero->AwardXP(XPReward);
				}
			}
		}

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCharacterMovement()->DisableMovement();
		if (!bIsSummonedMinion)
		{
			SpawnDrops();
		}

		float MontageDuration = 0.f;
		if (DeathMontage)
		{
			MontageDuration = PlayAnimMontage(DeathMontage);
		}

		const float DestroyDelay = FMath::Max(MontageDuration, 0.1f) + 2.f;
		GetWorldTimerManager().SetTimer(DestroyTimerHandle, this,
			&ADiabloEnemy::OnDestroyTimer, DestroyDelay, false);
	}

	return ActualDamage;
}

void ADiabloEnemy::OnDestroyTimer()
{
	Destroy();
}

void ADiabloEnemy::StartAttack(AActor* Target)
{
	if (bIsAttacking || !AttackMontage)
	{
		return;
	}

	bIsAttacking = true;
	AttackTarget = Target;
	PlayAnimMontage(AttackMontage);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ADiabloEnemy::OnAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
	}
}

bool ADiabloEnemy::StartSpecialAttack(AActor* Target)
{
	if (!Target || !CanUsePrimaryAttack())
	{
		return false;
	}

	if (IsSummonerArchetype() && TrySummonMinion(Target))
	{
		MarkPrimaryAttackUsed();
		return true;
	}

	if (IsRangedArchetype() || IsCasterArchetype() || IsSummonerArchetype())
	{
		if (FireProjectileAt(Target))
		{
			MarkPrimaryAttackUsed();
			return true;
		}
	}

	StartAttack(Target);
	MarkPrimaryAttackUsed();
	return true;
}

bool ADiabloEnemy::TrySummonMinion(AActor* Target)
{
	if (!SummonClass || MaxSummons <= 0 || GetAliveSummonCount() >= MaxSummons)
	{
		return false;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (Now - LastSummonTime < SummonCooldown)
	{
		return false;
	}

	const FVector ToTarget = Target ?
		(Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D() :
		GetActorForwardVector();
	const FVector Side(-ToTarget.Y, ToTarget.X, 0.f);
	const FVector SpawnLoc = GetActorLocation() - ToTarget * SummonRadius +
		Side * FMath::RandRange(-SummonRadius * 0.5f, SummonRadius * 0.5f) +
		FVector(0.f, 0.f, 40.f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.Owner = this;

	ADiabloEnemy* Minion = GetWorld()->SpawnActor<ADiabloEnemy>(
		SummonClass, SpawnLoc, FRotator::ZeroRotator, Params);
	if (!Minion)
	{
		return false;
	}

	Minion->ConfigureArchetype(EDiabloEnemyArchetype::MeleeGrunt);
	Minion->bIsSummonedMinion = true;
	Minion->Stats.MaxHP = FMath::Max(8.f, Stats.MaxHP * 0.4f);
	Minion->Stats.HP = Minion->Stats.MaxHP;
	Minion->XPReward = 0;
	Minion->DropTable.Empty();
	SpawnedSummons.Add(Minion);
	LastSummonTime = Now;

	UE_LOG(LogDiablo, Display, TEXT("%s summoned %s"), *GetName(), *Minion->GetName());
	return true;
}

void ADiabloEnemy::ConfigureArchetype(EDiabloEnemyArchetype NewArchetype)
{
	Archetype = NewArchetype;
	ApplyArchetypeDefaults();
}

void ADiabloEnemy::ApplyArchetypeDefaults()
{
	AggroRange = 800.f;
	AttackRange = 200.f;
	PreferredRange = 0.f;
	LeashRange = 1500.f;
	AttackCooldown = 1.2f;
	FleeHealthPercent = 0.f;
	FleeDuration = 2.f;
	ProjectileDamage = 8.f;
	ProjectileSpeed = 900.f;
	MaxSummons = 0;
	SummonCooldown = 6.f;
	SummonRadius = 250.f;

	switch (Archetype)
	{
	case EDiabloEnemyArchetype::FastMelee:
		Stats.MaxHP = 22.f;
		Stats.HP = Stats.MaxHP;
		AttackCooldown = 0.8f;
		GetCharacterMovement()->MaxWalkSpeed = 520.f;
		break;
	case EDiabloEnemyArchetype::RangedArcher:
		Stats.MaxHP = 24.f;
		Stats.HP = Stats.MaxHP;
		AttackRange = 900.f;
		PreferredRange = 550.f;
		AttackCooldown = 1.6f;
		ProjectileDamage = 7.f;
		ProjectileSpeed = 1200.f;
		GetCharacterMovement()->MaxWalkSpeed = 360.f;
		break;
	case EDiabloEnemyArchetype::Spellcaster:
		Stats.MaxHP = 20.f;
		Stats.HP = Stats.MaxHP;
		AttackRange = 1000.f;
		PreferredRange = 650.f;
		AttackCooldown = 2.2f;
		ProjectileDamage = 14.f;
		ProjectileSpeed = 850.f;
		GetCharacterMovement()->MaxWalkSpeed = 340.f;
		break;
	case EDiabloEnemyArchetype::Summoner:
		Stats.MaxHP = 34.f;
		Stats.HP = Stats.MaxHP;
		AttackRange = 850.f;
		PreferredRange = 600.f;
		AttackCooldown = 2.5f;
		ProjectileDamage = 6.f;
		ProjectileSpeed = 800.f;
		MaxSummons = 2;
		SummonCooldown = 7.f;
		GetCharacterMovement()->MaxWalkSpeed = 320.f;
		break;
	case EDiabloEnemyArchetype::FallenCoward:
		Stats.MaxHP = 18.f;
		Stats.HP = Stats.MaxHP;
		FleeHealthPercent = 0.35f;
		FleeDuration = 2.5f;
		AttackCooldown = 1.f;
		GetCharacterMovement()->MaxWalkSpeed = 440.f;
		break;
	case EDiabloEnemyArchetype::MeleeGrunt:
	default:
		Stats.MaxHP = 30.f;
		Stats.HP = Stats.MaxHP;
		GetCharacterMovement()->MaxWalkSpeed = 400.f;
		break;
	}
}

float ADiabloEnemy::GetHealthPercent() const
{
	return Stats.MaxHP > 0.f ? Stats.HP / Stats.MaxHP : 0.f;
}

bool ADiabloEnemy::IsRangedArchetype() const
{
	return Archetype == EDiabloEnemyArchetype::RangedArcher;
}

bool ADiabloEnemy::IsCasterArchetype() const
{
	return Archetype == EDiabloEnemyArchetype::Spellcaster;
}

bool ADiabloEnemy::IsSummonerArchetype() const
{
	return Archetype == EDiabloEnemyArchetype::Summoner;
}

bool ADiabloEnemy::ShouldFlee() const
{
	return FleeHealthPercent > 0.f && GetHealthPercent() <= FleeHealthPercent;
}

bool ADiabloEnemy::CanUsePrimaryAttack() const
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	return Now - LastPrimaryAttackTime >= AttackCooldown;
}

void ADiabloEnemy::MarkPrimaryAttackUsed()
{
	LastPrimaryAttackTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
}

void ADiabloEnemy::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
}

void ADiabloEnemy::SpawnDrops()
{
	if (DropTable.Num() == 0) return;

	for (const FDropTableEntry& Entry : DropTable)
	{
		if (!Entry.ItemDef) continue;
		if (FMath::FRand() > Entry.DropChance) continue;

		FVector SpawnLoc = GetActorLocation();
		SpawnLoc.Z += 10.f;
		// Random offset so multiple drops don't stack
		SpawnLoc.X += FMath::RandRange(-50.f, 50.f);
		SpawnLoc.Y += FMath::RandRange(-50.f, 50.f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ADroppedItem* Drop = GetWorld()->SpawnActor<ADroppedItem>(
			ADroppedItem::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);

		if (Drop)
		{
			FItemInstance Item;
			Item.Definition = Entry.ItemDef;
			Item.CurrentDurability = Entry.ItemDef->MaxDurability;
			Item.StackCount = 1;
			FAffixGenerator::TryMakeMagic(Item, MonsterLevel);
			Drop->InitFromItem(Item);

			UE_LOG(LogDiablo, Display, TEXT("%s dropped %s"),
				*GetName(), *Entry.ItemDef->DisplayName.ToString());
		}
	}
}

bool ADiabloEnemy::FireProjectileAt(AActor* Target)
{
	if (!Target || !ProjectileClass)
	{
		return false;
	}

	FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		Direction = GetActorForwardVector();
	}

	SetActorRotation(Direction.Rotation());

	const FVector SpawnLoc = GetActorLocation() + Direction * 80.f + FVector(0.f, 0.f, 50.f);
	FActorSpawnParameters Params;
	Params.Instigator = this;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASpellProjectile* Projectile = GetWorld()->SpawnActor<ASpellProjectile>(
		ProjectileClass, SpawnLoc, Direction.Rotation(), Params);
	if (!Projectile)
	{
		return false;
	}

	Projectile->Damage = ProjectileDamage;
	Projectile->Speed = ProjectileSpeed;
	Projectile->bDamageEnemies = false;
	Projectile->bDamageHero = true;
	if (Projectile->ProjectileMovement)
	{
		Projectile->ProjectileMovement->InitialSpeed = ProjectileSpeed;
		Projectile->ProjectileMovement->MaxSpeed = ProjectileSpeed;
		Projectile->ProjectileMovement->Velocity = Direction * ProjectileSpeed;
	}

	UE_LOG(LogDiablo, Display, TEXT("%s fired a projectile at %s"), *GetName(), *Target->GetName());
	return true;
}

int32 ADiabloEnemy::GetAliveSummonCount()
{
	SpawnedSummons.RemoveAll([](const TWeakObjectPtr<ADiabloEnemy>& Summon)
	{
		return !Summon.IsValid() || Summon->IsDead();
	});

	return SpawnedSummons.Num();
}
