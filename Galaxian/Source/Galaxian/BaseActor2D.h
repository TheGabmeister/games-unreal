#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseActor2D.generated.h"

class UPaperSpriteComponent;

UCLASS()
class GALAXIAN_API ABaseActor2D : public AActor
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UPaperSpriteComponent* SpriteComp;

public:	
	
	ABaseActor2D();

};
