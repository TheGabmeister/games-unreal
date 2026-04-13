#include "QuakeTrigger_Spawn.h"

#include "QuakeEnemySpawnPoint.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeSpawnTrigger, Log, All);

void AQuakeTrigger_Spawn::Activate(AActor* InInstigator)
{
	for (AQuakeEnemySpawnPoint* SP : SpawnPoints)
	{
		if (!SP)
		{
			UE_LOG(LogQuakeSpawnTrigger, Warning,
				TEXT("%s: null entry in SpawnPoints — authoring error, fix in the editor."),
				*GetName());
			continue;
		}
		SP->Activate(InInstigator);
	}

	Super::Activate(InInstigator);
}
