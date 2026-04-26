#include "DiabloEnemy.h"
#include "DiabloHero.h"
#include "DiabloAIController.h"
#include "DroppedItem.h"
#include "ItemDefinition.h"
#include "AffixGenerator.h"
#include "Diablo.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"

ADiabloEnemy::ADiabloEnemy()
{
	Stats.HP = 30.f;
	Stats.MaxHP = 30.f;
	XPReward = 2000;

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

		if (ADiabloHero* Hero = Cast<ADiabloHero>(UGameplayStatics::GetPlayerPawn(this, 0)))
		{
			const int32 LevelDiff = Hero->CharLevel - MonsterLevel;
			if (LevelDiff < 10)
			{
				Hero->AwardXP(XPReward);
			}
		}

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCharacterMovement()->DisableMovement();
		SpawnDrops();

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
