#include "QuakeTrigger_Hurt.h"

#include "QuakeDamageType_Telefrag.h"

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AQuakeTrigger_Hurt::AQuakeTrigger_Hurt()
{
	DamageTypeClass = UQuakeDamageType_Telefrag::StaticClass();

	// Kill floors / crusher pits bite enemies too — matches original Quake.
	bAllowMonsters = true;
}

void AQuakeTrigger_Hurt::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerVolume)
	{
		TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AQuakeTrigger_Hurt::OnVolumeEndOverlap);
	}
}

void AQuakeTrigger_Hurt::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		for (auto& Pair : ActiveTimers)
		{
			World->GetTimerManager().ClearTimer(Pair.Value);
		}
	}
	ActiveTimers.Reset();

	Super::EndPlay(EndPlayReason);
}

void AQuakeTrigger_Hurt::Activate(AActor* InInstigator)
{
	// Self-contained: starts a damage timer for the overlapping pawn. Does
	// NOT fire Targets (SPEC 5.6). EndOverlap stops the timer.
	if (!InInstigator)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerHandle& Handle = ActiveTimers.FindOrAdd(InInstigator);
	TWeakObjectPtr<AActor> WeakPawn = InInstigator;
	FTimerDelegate Delegate = FTimerDelegate::CreateUObject(
		this, &AQuakeTrigger_Hurt::ApplyTickDamage, WeakPawn);
	World->GetTimerManager().SetTimer(Handle, Delegate, TickRate, /*bLoop*/ true);

	// Fire one tick immediately so the trap bites on entry.
	ApplyTickDamage(WeakPawn);
}

void AQuakeTrigger_Hurt::OnVolumeEndOverlap(
	UPrimitiveComponent* /*OverlappedComp*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	int32 /*OtherBodyIndex*/)
{
	if (!OtherActor)
	{
		return;
	}
	if (FTimerHandle* Existing = ActiveTimers.Find(OtherActor))
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(*Existing);
		}
		ActiveTimers.Remove(OtherActor);
	}
}

void AQuakeTrigger_Hurt::ApplyTickDamage(TWeakObjectPtr<AActor> Pawn)
{
	AActor* Target = Pawn.Get();
	if (!Target)
	{
		return;
	}

	UGameplayStatics::ApplyPointDamage(
		Target,
		DamagePerTick,
		/*HitFromDirection*/ FVector::UpVector,
		FHitResult{},
		/*EventInstigator*/ nullptr,
		/*DamageCauser*/ this,
		DamageTypeClass);
}
