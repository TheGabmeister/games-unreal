#pragma once

#include "CoreMinimal.h"
#include "QuakeTrigger.h"
#include "QuakeTrigger_Spawn.generated.h"

/**
 * Phase 8 spawn trigger per SPEC section 5.6. Placeholder for Phase 9 —
 * on activation, the spawn trigger will iterate a typed TArray of
 * AQuakeEnemySpawnPoint references and tell each to spawn its enemy
 * class.
 *
 * AQuakeEnemySpawnPoint does not exist yet (lands in Phase 9 Spawn Points
 * + Stats + Level Transitions). For Phase 8 the class only declares its
 * shell so buttons and relays can reference it in the editor; Activate()
 * falls back to the base Targets chain.
 */
UCLASS()
class QUAKE_API AQuakeTrigger_Spawn : public AQuakeTrigger
{
	GENERATED_BODY()

	// TODO Phase 9: add UPROPERTY TArray<TObjectPtr<AQuakeEnemySpawnPoint>>
	// SpawnPoints and override Activate to iterate + call Spawn() on each.
};
