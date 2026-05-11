#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "AssetImportCommandlet.generated.h"

UCLASS()
class UAssetImportCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    virtual int32 Main(const FString& Params) override;
};
