#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "QuakeSaveable.generated.h"

/**
 * Phase 11 save-participation interface per DESIGN 6.2.
 *
 * Every actor that owns persistent level state (doors, buttons, secrets,
 * spawn points, Character HP) implements this. The GameMode walks
 * TActorIterator<AActor> at save/load time, gates on
 * Actor->Implements<UQuakeSaveable>(), and calls SaveState / LoadState on
 * the typed interface.
 *
 * **Pure C++ virtual, NOT a BlueprintNativeEvent.** Same rule as
 * IQuakeActivatable — do not write _Implementation suffix methods here;
 * subclasses override SaveState / LoadState directly. Parameter names use
 * the In/Out prefix convention to avoid the `Instigator` shadow warning
 * (AActor has a protected Instigator member).
 *
 * Stable identity is AActor::GetFName() — the editor-assigned FName
 * serialized with the .umap. Runtime-spawned actors have unstable FNames
 * and are intentionally excluded from persistence (DESIGN 6.2: spawn
 * points re-fire on level reload and reproduce enemies).
 */

USTRUCT()
struct QUAKE_API FActorSaveRecord
{
	GENERATED_BODY()

	/** Owner's GetFName() — matched against level actors on load. */
	UPROPERTY()
	FName ActorName;

	/** FMemoryWriter blob from the actor's SaveState, filtered by ArIsSaveGame. */
	UPROPERTY()
	TArray<uint8> Payload;
};

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UQuakeSaveable : public UInterface
{
	GENERATED_BODY()
};

class QUAKE_API IQuakeSaveable
{
	GENERATED_BODY()

public:
	/**
	 * Serialize this actor's persistent state into OutRecord.Payload. Set
	 * OutRecord.ActorName = GetFName(). Most implementations can delegate
	 * the payload work to WriteActorSaveProperties (QuakeSaveArchive.h).
	 */
	virtual void SaveState(FActorSaveRecord& OutRecord) = 0;

	/**
	 * Restore state from InRecord.Payload. Called by the GameMode when an
	 * FName match is found. Unmatched actors keep their level defaults —
	 * they never see LoadState.
	 */
	virtual void LoadState(const FActorSaveRecord& InRecord) = 0;
};
