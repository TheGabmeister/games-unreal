#include "PortalActor.h"
#include "DiabloHero.h"
#include "DiabloGameInstance.h"
#include "Diablo.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

APortalActor::APortalActor()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CylinderMesh.Object);
	}
}

void APortalActor::Interact(AActor* Interactor)
{
	ADiabloHero* Hero = Cast<ADiabloHero>(Interactor);
	if (!Hero) return;

	Hero->SaveToGameInstance();

	UDiabloGameInstance* GI = Cast<UDiabloGameInstance>(GetGameInstance());
	if (!GI) return;

	if (bReturnsToDungeon)
	{
		GI->CurrentFloorIndex = GI->PortalFloorIndex;
		UE_LOG(LogDiablo, Display, TEXT("Portal: returning to dungeon floor %d"), GI->PortalFloorIndex);
		UGameplayStatics::OpenLevel(this, FName("Lvl_Dungeon"));
	}
	else
	{
		GI->CurrentFloorIndex = 0;
		UE_LOG(LogDiablo, Display, TEXT("Portal: teleporting to town"));
		UGameplayStatics::OpenLevel(this, FName("Lvl_Diablo"));
	}
}
