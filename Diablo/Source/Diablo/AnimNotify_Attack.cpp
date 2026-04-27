#include "AnimNotify_Attack.h"
#include "DiabloHero.h"
#include "DiabloEnemy.h"
#include "InventoryComponent.h"
#include "Diablo.h"
#include "Engine/DamageEvents.h"

void UAnimNotify_Attack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner)
	{
		return;
	}

	AActor* Target = nullptr;
	AController* Instigator = nullptr;

	if (ADiabloHero* Hero = Cast<ADiabloHero>(Owner))
	{
		Target = Hero->AttackTarget;
		Instigator = Hero->GetController();
	}
	else if (ADiabloEnemy* Enemy = Cast<ADiabloEnemy>(Owner))
	{
		Target = Enemy->AttackTarget;
		Instigator = Enemy->GetController();
	}

	if (!Target)
	{
		return;
	}

	if (ADiabloHero* HeroTarget = Cast<ADiabloHero>(Target))
	{
		if (HeroTarget->IsDead()) { return; }
	}
	else if (ADiabloEnemy* EnemyTarget = Cast<ADiabloEnemy>(Target))
	{
		if (EnemyTarget->IsDead()) { return; }
	}

	// --- Hero attacking enemy: To-Hit% + Str-based damage ---
	if (ADiabloHero* Hero = Cast<ADiabloHero>(Owner))
	{
		if (ADiabloEnemy* EnemyTarget = Cast<ADiabloEnemy>(Target))
		{
			// D1 To-Hit: 50 + Dex/2 + CharLevel - MonsterAC + (CharLevel - MonsterLevel)
			const float ToHit = FMath::Clamp(
				50.f + Hero->Stats.Dex / 2.f + static_cast<float>(Hero->CharLevel)
				- EnemyTarget->MonsterAC
				+ static_cast<float>(Hero->CharLevel - EnemyTarget->MonsterLevel),
				5.f, 95.f);

			const float Roll = FMath::FRandRange(0.f, 100.f);
			if (Roll > ToHit)
			{
				UE_LOG(LogDiablo, Display, TEXT("MISS (roll %.0f > toHit %.0f%%)"), Roll, ToHit);
				return;
			}

			float BaseDamage;
			if (Hero->EquipMaxDamage > 0.f)
			{
				BaseDamage = FMath::FRandRange(Hero->EquipMinDamage, Hero->EquipMaxDamage);
			}
			else
			{
				BaseDamage = Damage;
			}
			const float FinalDamage = BaseDamage + Hero->Stats.Str / 5.f;
			UE_LOG(LogDiablo, Display, TEXT("HIT (roll %.0f <= toHit %.0f%%) damage %.0f (base %.0f + Str/5)"),
				Roll, ToHit, FinalDamage, BaseDamage);

			FDamageEvent DamageEvent;
			Target->TakeDamage(FinalDamage, DamageEvent, Instigator, Owner);
			if (Hero->Inventory)
			{
				Hero->Inventory->DegradeWeapon();
				Hero->RecomputeDerivedStats();
				Hero->OnStatsChanged.Broadcast();
			}
			return;
		}
	}

	// --- Enemy attacking hero (or any other case): flat damage ---
	FDamageEvent DamageEvent;
	Target->TakeDamage(Damage, DamageEvent, Instigator, Owner);
}

FString UAnimNotify_Attack::GetNotifyName_Implementation() const
{
	return TEXT("Melee Attack Trace");
}
