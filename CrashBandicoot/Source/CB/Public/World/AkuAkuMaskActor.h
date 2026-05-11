#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AkuAkuMaskActor.generated.h"

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AAkuAkuMaskActor : public AActor
{
    GENERATED_BODY()
public:
    AAkuAkuMaskActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CB")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TObjectPtr<UMaterialInterface> NormalMaterial;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TObjectPtr<UMaterialInterface> GoldenMaterial;

    void SetNormalAppearance();
    void SetGoldenAppearance();
};
