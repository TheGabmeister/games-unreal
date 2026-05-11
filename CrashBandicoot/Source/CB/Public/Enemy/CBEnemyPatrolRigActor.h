

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CBEnemyPatrolRigActor.generated.h"

class UCBEnemyPatrolRigComponent;

/*
* This is an example of an actor class with a default subobject generated at construction time. This is useful
* for guaranteeing there is a required component placed into the hierarchy that cannot be removed.
* 
* This essentially wraps our CBEnemyPatrolRigComponent to allow it to be placed anywhere in
* the level. This provides additional flexibility, as we can either place this actor into the level to
* place a patrol at any arbitrary location, or attach the component directly to an object instance
* that already exists in the level.
*/
UCLASS()
class CB_API ACBEnemyPatrolRigActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Default constructor 
	ACBEnemyPatrolRigActor();

public:	
	// The created patrol rig component that this actor owns 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCBEnemyPatrolRigComponent> PatrolRigComponent;
};
