#pragma once

#include "CoreMinimal.h"
#include "QuakeTrigger.h"
#include "QuakeTrigger_Spawn.generated.h"

class AQuakeEnemySpawnPoint;

/**
 * Spawn trigger per SPEC section 5.6. On activation, iterates the typed
 * SpawnPoints array and calls Activate() on each — deferred spawn points
 * treat Activate as their "spawn now" signal (see AQuakeEnemySpawnPoint).
 * Then fires the base Targets chain so relays / messages / etc. wired to
 * this same trigger still run.
 *
 * The typed SpawnPoints array is separate from the base Targets array so
 * the editor's actor picker filters the dropdown to AQuakeEnemySpawnPoint
 * only — catches authoring typos at edit time instead of runtime.
 */
UCLASS()
class QUAKE_API AQuakeTrigger_Spawn : public AQuakeTrigger
{
	GENERATED_BODY()

public:
	/** Spawn points to fire on activation. Typed for editor picker filtering. */
	UPROPERTY(EditInstanceOnly, Category = "Trigger|Spawn")
	TArray<TObjectPtr<AQuakeEnemySpawnPoint>> SpawnPoints;

	virtual void Activate(AActor* InInstigator) override;
};
