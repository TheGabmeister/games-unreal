#include "DroppedItem.h"
#include "DiabloHero.h"
#include "Diablo.h"
#include "Components/StaticMeshComponent.h"

ADroppedItem::ADroppedItem()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ADroppedItem::OnPickedUp(ADiabloHero* Hero)
{
	if (!Hero || Hero->IsDead())
	{
		return;
	}

	UE_LOG(LogDiablo, Display, TEXT("%s picked up %s (heal %.0f HP)"),
		*Hero->GetName(), *GetName(), HealAmount);

	Hero->Heal(HealAmount);
	Destroy();
}
