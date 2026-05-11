#include "Projectiles/BeakerProjectile.h"

#include "CBCollisionChannels.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "Enemy/GreenBlobEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CBPlayerCharacter.h"

ABeakerProjectile::ABeakerProjectile()
{
	Speed = 600.0f;
	ArcGravity = 980.0f;
}

void ABeakerProjectile::OnImpact(const FHitResult& Hit)
{
	FVector ImpactLocation = Hit.ImpactPoint;

	if (BeakerType == EBeakerType::Green)
	{
		if (BlobClass)
		{
			FTransform SpawnTransform(FRotator::ZeroRotator, ImpactLocation);
			GetWorld()->SpawnActor<AGreenBlobEnemy>(BlobClass, SpawnTransform);
		}
	}
	else
	{
		// Red beaker: brief damaging overlap sphere
		TArray<FOverlapResult> Overlaps;
		FCollisionShape Shape = FCollisionShape::MakeSphere(RedExplosionRadius);
		GetWorld()->OverlapMultiByChannel(Overlaps, ImpactLocation, FQuat::Identity, ECC_Pawn, Shape);

		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (ACBPlayerCharacter* Player = Cast<ACBPlayerCharacter>(Overlap.GetActor()))
			{
				if (!Player->IsDead())
				{
					Player->OnHit(this);
				}
			}
		}
	}

	Destroy();
}
