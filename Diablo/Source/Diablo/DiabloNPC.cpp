#include "DiabloNPC.h"
#include "DiabloPlayerController.h"
#include "Diablo.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ADiabloNPC::ADiabloNPC()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}
}

void ADiabloNPC::Interact(AActor* Interactor)
{
	if (APawn* Pawn = Cast<APawn>(Interactor))
	{
		if (ADiabloPlayerController* PC = Cast<ADiabloPlayerController>(Pawn->GetController()))
		{
			PC->ShowDialog(NPCName, DialogText);
		}
	}
}
