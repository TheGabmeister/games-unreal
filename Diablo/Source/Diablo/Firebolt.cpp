#include "Firebolt.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"

AFirebolt::AFirebolt()
{
	ManaCost = 6.f;
	Cooldown = 0.8f;
	Damage = 20.f;
	Speed = 1200.f;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMesh.Object);
	}
}
