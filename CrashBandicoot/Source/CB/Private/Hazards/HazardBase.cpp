#include "Hazards/HazardBase.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CBPlayerCharacter.h"

AHazardBase::AHazardBase()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DamageVolume = CreateDefaultSubobject<USphereComponent>(TEXT("DamageVolume"));
	DamageVolume->SetupAttachment(MeshComponent);
	DamageVolume->SetSphereRadius(50.0f);
	DamageVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	DamageVolume->SetGenerateOverlapEvents(true);

	DamageVolume->OnComponentBeginOverlap.AddDynamic(this, &AHazardBase::OnDamageOverlap);
}

void AHazardBase::OnDamageOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACBPlayerCharacter* Player = Cast<ACBPlayerCharacter>(Other))
	{
		if (!Player->IsDead())
		{
			Player->OnHit(this);
		}
	}
}
