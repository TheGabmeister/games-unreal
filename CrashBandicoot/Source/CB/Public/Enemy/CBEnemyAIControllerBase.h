

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CBEnemyAIControllerBase.generated.h"

UCLASS(Blueprintable, meta = (PrioritizeCategories = "CB"))
class CB_API ACBEnemyAIControllerBase : public AAIController
{
	GENERATED_BODY()
};
