#include "DiabloNPC.h"
#include "DiabloPlayerController.h"
#include "DiabloHero.h"
#include "InventoryComponent.h"
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
	APawn* Pawn = Cast<APawn>(Interactor);
	if (!Pawn) return;

	ADiabloPlayerController* PC = Cast<ADiabloPlayerController>(Pawn->GetController());
	if (!PC) return;

	switch (NPCType)
	{
	case ENPCType::Merchant:
		PC->OpenShop(this);
		break;

	case ENPCType::Healer:
		if (ADiabloHero* Hero = Cast<ADiabloHero>(Interactor))
		{
			Hero->Heal(Hero->Stats.MaxHP);
		}
		PC->ShowDialog(NPCName, FText::FromString(TEXT("Drink this, you look terrible.")));
		break;

	case ENPCType::Identifier:
	{
		ADiabloHero* Hero = Cast<ADiabloHero>(Interactor);
		if (Hero && Hero->Inventory)
		{
			const int32 Count = Hero->Inventory->IdentifyAll();
			if (Count > 0)
			{
				Hero->RecomputeDerivedStats();
				Hero->OnStatsChanged.Broadcast();
				PC->ShowDialog(NPCName, FText::FromString(
					FString::Printf(TEXT("I have identified %d item(s) for you."), Count)));
			}
			else
			{
				PC->ShowDialog(NPCName, FText::FromString(TEXT("You have nothing that needs identification.")));
			}
		}
		break;
	}

	default:
		PC->ShowDialog(NPCName, DialogText);
		break;
	}
}
