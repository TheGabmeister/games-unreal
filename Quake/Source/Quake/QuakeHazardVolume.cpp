#include "QuakeHazardVolume.h"

#include "QuakeDamageType_Lava.h"

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeHazard, Log, All);

AQuakeHazardVolume::AQuakeHazardVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	VolumeBox = CreateDefaultSubobject<UBoxComponent>(TEXT("VolumeBox"));
	SetRootComponent(VolumeBox);

	// Overlap only — never block, never occlude. Pawns walk into lava, not
	// against it. Visibility/weapon traces should not register hits either,
	// so set all responses to Ignore and then enable Pawn overlap.
	VolumeBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VolumeBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	VolumeBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	VolumeBox->SetGenerateOverlapEvents(true);

	// Default to lava. BP_HazardVolume_Slime overrides to UQuakeDamageType_Slime
	// + 4 dmg + 0 entry knockback when that damage type lands.
	DamageTypeClass = UQuakeDamageType_Lava::StaticClass();
}

void AQuakeHazardVolume::BeginPlay()
{
	Super::BeginPlay();

	if (VolumeBox)
	{
		VolumeBox->OnComponentBeginOverlap.AddDynamic(this, &AQuakeHazardVolume::OnVolumeBeginOverlap);
		VolumeBox->OnComponentEndOverlap.AddDynamic(this, &AQuakeHazardVolume::OnVolumeEndOverlap);
	}
}

void AQuakeHazardVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

void AQuakeHazardVolume::OnVolumeBeginOverlap(
	UPrimitiveComponent* /*OverlappedComp*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/,
	const FHitResult& /*SweepResult*/)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Entry knockback — SPEC 5.2: 200 u away from volume center on entry.
	// Horizontal only (bXYOverride = true, Z untouched). Skip cleanly when
	// strength is zero (slime).
	if (EntryKnockbackStrength > 0.f)
	{
		if (ACharacter* Char = Cast<ACharacter>(OtherActor))
		{
			const FVector Away = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
			if (!Away.IsNearlyZero())
			{
				Char->LaunchCharacter(Away * EntryKnockbackStrength, /*bXYOverride*/ true, /*bZOverride*/ false);
			}
		}
	}

	// Start the repeating damage timer. First tick fires at TickRate — no
	// instantaneous entry damage; matches original Quake.
	FTimerHandle& Handle = ActiveTimers.FindOrAdd(OtherActor);
	TWeakObjectPtr<AActor> WeakPawn = OtherActor;
	FTimerDelegate Delegate = FTimerDelegate::CreateUObject(
		this, &AQuakeHazardVolume::ApplyTickDamage, WeakPawn);
	World->GetTimerManager().SetTimer(Handle, Delegate, TickRate, /*bLoop*/ true);
}

void AQuakeHazardVolume::OnVolumeEndOverlap(
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

void AQuakeHazardVolume::ApplyTickDamage(TWeakObjectPtr<AActor> Pawn)
{
	AActor* Target = Pawn.Get();
	if (!Target)
	{
		// Target was destroyed while the timer was armed. The timer will
		// continue to fire until EndOverlap — but GetWorld-stored handles are
		// keyed by the now-stale weak pointer, so let the eventual EndPlay
		// sweep clean it up.
		return;
	}

	UGameplayStatics::ApplyPointDamage(
		Target,
		DamagePerTick,
		/*HitFromDirection*/ FVector::UpVector,
		FHitResult{},
		/*EventInstigator*/ nullptr,   // bCausedByWorld=true on the damage type
		/*DamageCauser*/ this,
		DamageTypeClass);
}
