#include "QuakeWeaponBase.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AQuakeWeaponBase::AQuakeWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ViewModelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ViewModelMesh"));
	RootComponent = ViewModelMesh;

	// The viewmodel never collides — it's a visual attachment to the camera.
	ViewModelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ViewModelMesh->SetGenerateOverlapEvents(false);
	ViewModelMesh->SetCastShadow(false);
}

bool AQuakeWeaponBase::CanFire() const
{
	if (LastFireWorldTime < 0.0)
	{
		return true;  // Never fired before — first shot is always free.
	}
	const double Cooldown = 1.0 / FMath::Max(RateOfFire, KINDA_SMALL_NUMBER);
	return (GetWorld()->GetTimeSeconds() - LastFireWorldTime) >= Cooldown;
}

bool AQuakeWeaponBase::TryFire(AActor* InInstigator)
{
	if (!CanFire())
	{
		return false;
	}
	LastFireWorldTime = GetWorld()->GetTimeSeconds();
	Fire(InInstigator);
	return true;
}
