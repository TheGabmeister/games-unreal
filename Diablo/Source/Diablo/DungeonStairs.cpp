#include "DungeonStairs.h"
#include "DiabloHero.h"
#include "Diablo.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ADungeonStairs::ADungeonStairs()
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

void ADungeonStairs::OnInteract()
{
	if (TargetLevelName.IsNone())
	{
		UE_LOG(LogDiablo, Warning, TEXT("DungeonStairs: TargetLevelName is not set"));
		return;
	}

	if (ADiabloHero* Hero = Cast<ADiabloHero>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Hero->SaveToGameInstance();
	}

	UE_LOG(LogDiablo, Display, TEXT("Transitioning to level: %s"), *TargetLevelName.ToString());
	UGameplayStatics::OpenLevel(this, TargetLevelName);
}
