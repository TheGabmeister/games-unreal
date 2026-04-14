#include "QuakeDoor.h"

#include "QuakeCharacter.h"
#include "QuakeDamageType_Telefrag.h"
#include "QuakeHUD.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeDoor, Log, All);

AQuakeDoor::AQuakeDoor()
{
	PrimaryActorTick.bCanEverTick = true;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	SetRootComponent(DoorMesh);

	// Closed door: block everything worldly. Pawns and projectiles cannot
	// cross; hitscan traces stop at the surface. Response matrix matches
	// SPEC section 1.6 for solid world geometry.
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorMesh->SetCollisionResponseToAllChannels(ECR_Block);
	DoorMesh->SetMobility(EComponentMobility::Movable);

	BlockingZone = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockingZone"));
	BlockingZone->SetupAttachment(DoorMesh);
	BlockingZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BlockingZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	BlockingZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BlockingZone->SetGenerateOverlapEvents(true);
}

void AQuakeDoor::BeginPlay()
{
	Super::BeginPlay();

	ClosedRelativeLoc = DoorMesh ? DoorMesh->GetRelativeLocation() : FVector::ZeroVector;
	OpenRelativeLoc = ClosedRelativeLoc + OpenAxis.GetSafeNormal() * OpenDistance;

	if (DoorMesh)
	{
		DoorMesh->OnComponentHit.AddDynamic(this, &AQuakeDoor::OnDoorHit);
	}
}

void AQuakeDoor::Activate(AActor* InInstigator)
{
	if (!CanOpenFor(InInstigator))
	{
		// SPEC 10 locked feedback: show the HUD message "You need the [color]
		// key" for 2 s.
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			if (AQuakeHUD* HUD = Cast<AQuakeHUD>(PC->GetHUD()))
			{
				const FText Msg = RequiredKey == EQuakeKeyColor::Silver
					? NSLOCTEXT("QuakeDoor", "NeedSilverKey", "You need the silver key.")
					: (RequiredKey == EQuakeKeyColor::Gold
						? NSLOCTEXT("QuakeDoor", "NeedGoldKey", "You need the gold key.")
						: NSLOCTEXT("QuakeDoor", "NeedKey", "You need a key."));
				HUD->ShowMessage(Msg, 2.f);
			}
		}
		UE_LOG(LogQuakeDoor, Log, TEXT("%s: locked — instigator %s lacks required key."),
			*GetName(), *GetNameSafe(InInstigator));
		return;
	}

	if (State == EState::Closed || State == EState::Closing)
	{
		State = EState::Opening;

		// Cancel any pending auto-close — we're going the other way now.
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CloseTimerHandle);
		}
	}
}

void AQuakeDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!DoorMesh)
	{
		return;
	}

	switch (State)
	{
	case EState::Opening:
	{
		const FVector Current = DoorMesh->GetRelativeLocation();
		const FVector ToOpen = OpenRelativeLoc - Current;
		const float DistRemaining = ToOpen.Size();
		const float Step = OpenSpeed * DeltaSeconds;

		if (Step >= DistRemaining)
		{
			DoorMesh->SetRelativeLocation(OpenRelativeLoc);
			State = EState::Open;

			// Arm the auto-close timer. 0 = stay open permanently.
			if (AutoCloseDelay > 0.f)
			{
				GetWorldTimerManager().SetTimer(CloseTimerHandle, this, &AQuakeDoor::TryStartClosing,
					AutoCloseDelay, false);
			}
		}
		else
		{
			DoorMesh->SetRelativeLocation(Current + ToOpen.GetSafeNormal() * Step);
		}
		break;
	}
	case EState::Closing:
	{
		const FVector Current = DoorMesh->GetRelativeLocation();
		const FVector ToClosed = ClosedRelativeLoc - Current;
		const float DistRemaining = ToClosed.Size();
		const float Step = CloseSpeed * DeltaSeconds;

		// Sweep move — collisions along the way fire OnComponentHit which
		// applies crush damage in OnDoorHit. The move still completes.
		const FVector NextLoc = (Step >= DistRemaining)
			? ClosedRelativeLoc
			: Current + ToClosed.GetSafeNormal() * Step;
		DoorMesh->SetRelativeLocation(NextLoc, /*bSweep*/ true);

		if (Step >= DistRemaining)
		{
			State = EState::Closed;
		}
		break;
	}
	default:
		break;
	}
}

void AQuakeDoor::TryStartClosing()
{
	if (State != EState::Open)
	{
		return;
	}

	if (IsBlockingZoneOccupied())
	{
		// Someone's in the doorway — wait a beat and try again.
		GetWorldTimerManager().SetTimer(CloseTimerHandle, this, &AQuakeDoor::TryStartClosing,
			0.5f, false);
		return;
	}

	State = EState::Closing;
}

bool AQuakeDoor::IsBlockingZoneOccupied() const
{
	if (!BlockingZone)
	{
		return false;
	}
	TArray<AActor*> Overlapping;
	BlockingZone->GetOverlappingActors(Overlapping, ACharacter::StaticClass());
	return Overlapping.Num() > 0;
}

bool AQuakeDoor::CanOpenFor(AActor* InInstigator) const
{
	if (RequiredKey == EQuakeKeyColor::None)
	{
		return true;
	}
	if (const AQuakeCharacter* Character = Cast<AQuakeCharacter>(InInstigator))
	{
		return Character->HasKey(RequiredKey);
	}
	// Non-player activator (trigger relay, enemy overlap) — refuse. Keeps
	// "a dying Grunt brushed the door open" from happening by accident.
	return false;
}

void AQuakeDoor::OnDoorHit(
	UPrimitiveComponent* /*HitComponent*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	FVector /*NormalImpulse*/,
	const FHitResult& Hit)
{
	// Only crush during Closing — opening-direction contacts happen when a
	// pawn leans against an opening door and should not kill anyone.
	if (State != EState::Closing || !OtherActor)
	{
		return;
	}
	if (!OtherActor->IsA(ACharacter::StaticClass()))
	{
		return;
	}

	UGameplayStatics::ApplyPointDamage(
		OtherActor,
		CrushDamage,
		/*HitFromDirection*/ -Hit.ImpactNormal,
		Hit,
		/*EventInstigator*/ nullptr,
		/*DamageCauser*/ this,
		UQuakeDamageType_Telefrag::StaticClass());

	UE_LOG(LogQuakeDoor, Log, TEXT("%s: crushed %s"), *GetName(), *OtherActor->GetName());
}
