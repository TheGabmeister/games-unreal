#include "QuakeTrigger_Exit.h"

#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuakeExit, Log, All);

void AQuakeTrigger_Exit::Activate(AActor* InInstigator)
{
	// Fire the chain first so any on-exit bookkeeping happens before the
	// world swap wipes everything.
	Super::Activate(InInstigator);

	if (NextMapName.IsNone())
	{
		UE_LOG(LogQuakeExit, Warning,
			TEXT("%s: NextMapName is None — level complete (stub)."), *GetName());
		return;
	}

	UE_LOG(LogQuakeExit, Log, TEXT("%s: exiting to %s"), *GetName(), *NextMapName.ToString());
	UGameplayStatics::OpenLevel(this, NextMapName);
}
