#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BounceCrate.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class CRASHBANDICOOT_API ABounceCrate : public AActor
{
    GENERATED_BODY()

public:
    ABounceCrate();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crate", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* Collision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crate", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crate")
    float BounceStrength = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crate")
    USoundBase* BounceSound;

    UFUNCTION()
    void OnCrateHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit);
};