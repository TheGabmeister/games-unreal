#pragma once

#include "CoreMinimal.h"
#include "SpellProjectile.h"
#include "Fireball.generated.h"

UCLASS()
class DIABLO_API AFireball : public ASpellProjectile
{
	GENERATED_BODY()

public:
	AFireball();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
