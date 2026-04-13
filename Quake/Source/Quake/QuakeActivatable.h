#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "QuakeActivatable.generated.h"

/**
 * Phase 8 activation chain interface per SPEC section 5.5.
 *
 * Every fireable actor (doors, triggers, secrets, level exit) implements this
 * interface; buttons and triggers hold typed TArray<TObjectPtr<AActor>> lists
 * and call Activate() on each entry. Replaces Quake's targetname / target
 * string lookup — editor actor-picker UX, refactor-safe, no runtime name
 * registry, no silent typo failures.
 *
 * **Pure C++ virtual, NOT a BlueprintNativeEvent.** Do not write
 * Activate_Implementation — that suffix is only valid for BlueprintNativeEvent
 * methods, and using it here will not compile. Subclasses override Activate
 * with `virtual void Activate(AActor*) override;` directly, matching the
 * project's "no Blueprint gameplay logic" rule.
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UQuakeActivatable : public UInterface
{
	GENERATED_BODY()
};

class QUAKE_API IQuakeActivatable
{
	GENERATED_BODY()

public:
	/**
	 * Fire this actor.
	 * @param Instigator  The pawn that caused the activation (player usually).
	 *                    May be null for indirect chains (e.g. relay fires relay).
	 */
	virtual void Activate(AActor* InInstigator) = 0;
};
