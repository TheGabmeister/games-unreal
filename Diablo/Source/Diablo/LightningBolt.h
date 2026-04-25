#pragma once

#include "CoreMinimal.h"
#include "SpellProjectile.h"
#include "LightningBolt.generated.h"

UCLASS()
class DIABLO_API ALightningBolt : public ASpellProjectile
{
	GENERATED_BODY()

public:
	ALightningBolt();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
