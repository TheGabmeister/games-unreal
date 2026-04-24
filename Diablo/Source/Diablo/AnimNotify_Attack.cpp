#include "AnimNotify_Attack.h"
#include "DiabloHero.h"
#include "DiabloEnemy.h"
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

	FDamageEvent DamageEvent;
	Target->TakeDamage(Damage, DamageEvent, Instigator, Owner);
}

FString UAnimNotify_Attack::GetNotifyName_Implementation() const
{
	return TEXT("Melee Attack Trace");
}
