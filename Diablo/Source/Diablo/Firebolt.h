#pragma once

#include "CoreMinimal.h"
#include "SpellProjectile.h"
#include "Firebolt.generated.h"

UCLASS()
class DIABLO_API AFirebolt : public ASpellProjectile
{
	GENERATED_BODY()

public:
	AFirebolt();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
