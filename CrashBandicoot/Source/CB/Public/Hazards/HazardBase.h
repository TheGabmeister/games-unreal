#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HazardBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ACBPlayerCharacter;

UCLASS(Abstract, meta = (PrioritizeCategories = "CB"))
class CB_API AHazardBase : public AActor
{
	GENERATED_BODY()

public:
	AHazardBase();

protected:
	UPROPERTY(VisibleAnywhere, Category = "CB")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "CB")
	TObjectPtr<USphereComponent> DamageVolume;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<USoundBase> HazardSound;

	UFUNCTION()
	void OnDamageOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
