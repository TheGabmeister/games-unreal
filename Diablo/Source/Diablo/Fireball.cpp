#include "Fireball.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"

AFireball::AFireball()
{
	ManaCost = 12.f;
	Cooldown = 1.2f;
	Damage = 40.f;
	Speed = 800.f;

	CollisionComponent->SetSphereRadius(40.f);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMesh.Object);
	}
}
