#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionInterfaces.generated.h"

class ACBPlayerCharacter;

UINTERFACE(MinimalAPI)
class USpinnable : public UInterface { GENERATED_BODY() };
class ISpinnable
{
    GENERATED_BODY()
public:
    virtual void OnSpinHit(ACBPlayerCharacter* Player) = 0;
};

UINTERFACE(MinimalAPI)
class UStompable : public UInterface { GENERATED_BODY() };
class IStompable
{
    GENERATED_BODY()
public:
    virtual void OnJumpHit(ACBPlayerCharacter* Player) = 0;
};

UINTERFACE(MinimalAPI)
class UExplodable : public UInterface { GENERATED_BODY() };
class IExplodable
{
    GENERATED_BODY()
public:
    virtual void OnExplosionHit(FVector Origin, float Radius) = 0;
};
