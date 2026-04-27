#pragma once

#include "CoreMinimal.h"
#include "SpellProjectile.h"
#include "ChainLightning.generated.h"

UCLASS()
class DIABLO_API AChainLightning : public ASpellProjectile
{
	GENERATED_BODY()

public:
	AChainLightning();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell")
	int32 BouncesRemaining = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell")
	float BounceRange = 600.f;

	UPROPERTY()
	TWeakObjectPtr<AActor> PreviousHitActor;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnChainOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
};
