// Crate.h
#pragma once
#include "CoreMinimal.h"
#include "Crates/CrateBase.h"
#include "Crate.generated.h"

UENUM(BlueprintType)
enum class ECrateContents : uint8
{
    Wumpa,
    WumpaLarge,
    Life,
    Mask,
    Token
};

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ACrate : public ACrateBase
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CB")
    ECrateContents Contents = ECrateContents::Wumpa;

protected:
    virtual void SpawnContents() override;
};
