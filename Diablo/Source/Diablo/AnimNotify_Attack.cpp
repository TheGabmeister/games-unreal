#include "AnimNotify_Attack.h"
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

	UWorld* World = Owner->GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Start = Owner->GetActorLocation();
	const FVector End = Start + Owner->GetActorForwardVector() * TraceLength;

	FCollisionShape Sphere = FCollisionShape::MakeSphere(TraceRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	FHitResult HitResult;
	if (World->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Pawn, Sphere, Params))
	{
		if (ADiabloEnemy* Enemy = Cast<ADiabloEnemy>(HitResult.GetActor()))
		{
			AController* Instigator = nullptr;
			if (APawn* Pawn = Cast<APawn>(Owner))
			{
				Instigator = Pawn->GetController();
			}

			FDamageEvent DamageEvent;
			Enemy->TakeDamage(Damage, DamageEvent, Instigator, Owner);
			UE_LOG(LogDiablo, Display, TEXT("Attack hit %s for %.0f damage"), *Enemy->GetName(), Damage);
		}
	}
}

FString UAnimNotify_Attack::GetNotifyName_Implementation() const
{
	return TEXT("Melee Attack Trace");
}
