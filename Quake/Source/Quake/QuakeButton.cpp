#include "QuakeButton.h"

#include "QuakeActivatable.h"
#include "QuakeCollisionChannels.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeButton, Log, All);

AQuakeButton::AQuakeButton()
{
	PrimaryActorTick.bCanEverTick = false;

	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));
	SetRootComponent(Collider);

	// Touch: pawn overlap. Shoot: weapon-channel trace hit. The button
	// supports both by overlapping Pawn and blocking the Weapon channel
	// at all times — ActivationMode gates which signal actually calls Fire
	// (see BeginPlay / OnColliderBeginOverlap / TakeDamage).
	Collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Collider->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collider->SetCollisionResponseToChannel(QuakeCollision::ECC_Weapon, ECR_Block);
	Collider->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Collider->SetGenerateOverlapEvents(true);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collider);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AQuakeButton::BeginPlay()
{
	Super::BeginPlay();

	if (Collider)
	{
		Collider->OnComponentBeginOverlap.AddDynamic(this, &AQuakeButton::OnColliderBeginOverlap);
	}
}

void AQuakeButton::OnColliderBeginOverlap(
	UPrimitiveComponent* /*OverlappedComp*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/,
	const FHitResult& /*SweepResult*/)
{
	if (!bArmed || ActivationMode != EQuakeButtonActivation::Touch)
	{
		return;
	}
	if (Cast<ACharacter>(OtherActor))
	{
		Fire(OtherActor);
	}
}

float AQuakeButton::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!bArmed || ActivationMode != EQuakeButtonActivation::Shoot || ActualDamage <= 0.f)
	{
		return ActualDamage;
	}

	// Attribute the activation to the firing pawn when available.
	AActor* PawnInstigator = EventInstigator ? EventInstigator->GetPawn() : DamageCauser;
	Fire(PawnInstigator);

	return ActualDamage;
}

void AQuakeButton::Fire(AActor* InInstigator)
{
	if (!bArmed)
	{
		return;
	}
	bArmed = false;

	for (AActor* Target : Targets)
	{
		if (!Target)
		{
			UE_LOG(LogQuakeButton, Warning,
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
			UE_LOG(LogQuakeButton, Warning,
				TEXT("%s: target %s does not implement IQuakeActivatable — skipped."),
				*GetName(), *Target->GetName());
		}
	}

	if (Cooldown > 0.f)
	{
		GetWorldTimerManager().SetTimer(CooldownHandle, this, &AQuakeButton::ReArm, Cooldown, false);
	}
	else
	{
		// One-shot: disable further activation entirely.
		if (Collider)
		{
			Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AQuakeButton::ReArm()
{
	bArmed = true;
}
