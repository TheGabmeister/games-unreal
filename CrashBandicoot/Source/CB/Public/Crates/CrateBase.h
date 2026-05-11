// CrateBase.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractionInterfaces.h"
#include "CrateBase.generated.h"

class USoundBase;

UCLASS(Abstract, meta = (PrioritizeCategories = "CB"))
class CB_API ACrateBase : public AActor, public ISpinnable, public IStompable, public IExplodable
{
    GENERATED_BODY()
public:
    ACrateBase();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CB")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(EditDefaultsOnly, Category = "CB")
    TObjectPtr<USoundBase> BreakSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CB")
    bool bCountsTowardTotal = true;

    // ISpinnable
    virtual void OnSpinHit(ACBPlayerCharacter* Player) override;
    // IStompable
    virtual void OnJumpHit(ACBPlayerCharacter* Player) override;
    // IExplodable
    virtual void OnExplosionHit(FVector Origin, float Radius) override;

protected:
    virtual void BreakCrate();
    virtual void SpawnContents();

    bool bIsBroken = false;
};
