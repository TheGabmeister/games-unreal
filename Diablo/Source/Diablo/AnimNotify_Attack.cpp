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
	ADiabloHero* Hero = Cast<ADiabloHero>(Owner);
	if (!Hero || !Hero->AttackTarget || Hero->AttackTarget->IsDead())
	{
		return;
	}

	AController* Instigator = Hero->GetController();
	FDamageEvent DamageEvent;
	Hero->AttackTarget->TakeDamage(Damage, DamageEvent, Instigator, Hero);
}

FString UAnimNotify_Attack::GetNotifyName_Implementation() const
{
	return TEXT("Melee Attack Trace");
}
