#include "DroppedItem.h"
#include "DiabloHero.h"
#include "InventoryComponent.h"
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

	if (ItemData.IsValid() && Hero->Inventory)
	{
		if (!Hero->Inventory->TryAddItem(ItemData))
		{
			UE_LOG(LogDiablo, Warning, TEXT("Inventory full — cannot pick up %s"),
				*ItemData.Definition->DisplayName.ToString());
			return;
		}
		UE_LOG(LogDiablo, Display, TEXT("%s picked up %s"),
			*Hero->GetName(), *ItemData.Definition->DisplayName.ToString());
		Destroy();
		return;
	}

	UE_LOG(LogDiablo, Display, TEXT("%s picked up %s (heal %.0f HP)"),
		*Hero->GetName(), *GetName(), HealAmount);
	Hero->Heal(HealAmount);
	Destroy();
}
