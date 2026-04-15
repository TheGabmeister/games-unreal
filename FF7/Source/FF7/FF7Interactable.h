// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FF7Interactable.generated.h"

class AFF7PlayerController;

/**
 * Two-class UINTERFACE pattern (SPEC §2.4). UHT generates `Execute_Interact`
 * on UFF7Interactable; callers use `IFF7Interactable::Execute_Interact(Obj, Interactor)`
 * which dispatches to the C++ or BP implementation transparently.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UFF7Interactable : public UInterface
{
	GENERATED_BODY()
};

class FF7_API IFF7Interactable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "FF7|Interact")
	void Interact(AFF7PlayerController* Interactor);
	virtual void Interact_Implementation(AFF7PlayerController* Interactor) {}
};
