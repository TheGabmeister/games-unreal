#include "LightningBolt.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"

ALightningBolt::ALightningBolt()
{
	ManaCost = 8.f;
	Cooldown = 0.5f;
	Damage = 15.f;
	Speed = 2000.f;

	CollisionComponent->SetSphereRadius(15.f);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.6f));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}
}
