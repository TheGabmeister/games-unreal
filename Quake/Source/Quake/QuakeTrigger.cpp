#include "QuakeTrigger.h"

#include "Components/BoxComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeTrigger, Log, All);

AQuakeTrigger::AQuakeTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	SetRootComponent(TriggerVolume);

	// Query-only overlap — triggers never block. Matches SPEC 1.6 Pickup
	// semantics (overlap Pawn, ignore everything else). Subclasses that
	// need a different response matrix override in their own ctor.
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerVolume->SetGenerateOverlapEvents(true);
}

void AQuakeTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AQuakeTrigger::OnTriggerBeginOverlap);
	}
}

void AQuakeTrigger::OnTriggerBeginOverlap(
	UPrimitiveComponent* /*OverlappedComp*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/,
	const FHitResult& /*SweepResult*/)
{
	if (OtherActor && OtherActor != this)
	{
		Activate(OtherActor);
	}
}

void AQuakeTrigger::Activate(AActor* InInstigator)
{
	// Base impl just fires Targets. Subclasses override to prepend or append
	// their own behavior, then call Super::Activate(InInstigator) to fire the
	// generic chain — or omit the Super call if the subclass is
	// self-contained (e.g. _Hurt).
	FireTargets(InInstigator);
}

void AQuakeTrigger::FireTargets(AActor* InInstigator)
{
	for (AActor* Target : Targets)
	{
		if (!Target)
		{
			UE_LOG(LogQuakeTrigger, Warning,
				TEXT("%s: null entry in Targets — authoring error, fix in the editor."),
				*GetName());
			continue;
		}
		if (IQuakeActivatable* Activatable = Cast<IQuakeActivatable>(Target))
		{
			Activatable->Activate(InInstigator);
		}
		else
		{
			UE_LOG(LogQuakeTrigger, Warning,
				TEXT("%s: target %s does not implement IQuakeActivatable — skipped."),
				*GetName(), *Target->GetName());
		}
	}
}
