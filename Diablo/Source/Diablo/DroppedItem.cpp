#include "DroppedItem.h"
#include "DiabloHero.h"
#include "InventoryComponent.h"
#include "Diablo.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ADroppedItem::ADroppedItem()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	static ConstructorHelpers::FObjectFinder<UMaterial> DropMatFinder(
		TEXT("/Game/Items/M_ItemDrop.M_ItemDrop"));
	if (DropMatFinder.Succeeded())
	{
		DropMaterial = DropMatFinder.Object;
	}
}

void ADroppedItem::InitFromItem(const FItemInstance& InItem)
{
	ItemData = InItem;

	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (!PlaneMesh || !MeshComponent) return;

	MeshComponent->SetStaticMesh(PlaneMesh);
	MeshComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
	MeshComponent->SetRelativeRotation(FRotator(90.f, 45.f, 0.f));

	if (DropMaterial && ItemData.Definition && ItemData.Definition->Icon)
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(DropMaterial, this);
		DynMat->SetTextureParameterValue(TEXT("Texture"), ItemData.Definition->Icon);
		MeshComponent->SetMaterial(0, DynMat);
	}
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
