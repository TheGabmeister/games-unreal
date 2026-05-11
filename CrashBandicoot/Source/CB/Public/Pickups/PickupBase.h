// PickupBase.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

class USphereComponent;
class USoundBase;
class ACBPlayerCharacter;

UCLASS(Abstract, meta = (PrioritizeCategories = "CB"))
class CB_API APickupBase : public AActor
{
    GENERATED_BODY()
public:
    APickupBase();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CB")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CB")
    TObjectPtr<USphereComponent> PickupTrigger;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TObjectPtr<USoundBase> PickupSound;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    virtual void OnPickedUp(ACBPlayerCharacter* Player);
    void Collect(ACBPlayerCharacter* Player);

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    float RotationSpeed = 90.0f;
};
