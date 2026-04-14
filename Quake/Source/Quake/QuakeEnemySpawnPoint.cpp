#include "QuakeEnemySpawnPoint.h"

#include "QuakeGameMode.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"

#if WITH_EDITORONLY_DATA
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogQuakeSpawnPoint, Log, All);

AQuakeEnemySpawnPoint::AQuakeEnemySpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// Empty SceneComponent as root gives the actor a movable transform
	// handle in the Editor — without a root the actor is still placeable
	// but awkward to select and rotate. Runtime-cheap (no rendering).
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

#if WITH_EDITORONLY_DATA
	EditorSprite = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorSprite"));
	if (EditorSprite)
	{
		EditorSprite->SetupAttachment(SceneRoot);
		EditorSprite->bIsEditorOnly = true;
	}

	EditorArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("EditorArrow"));
	if (EditorArrow)
	{
		EditorArrow->SetupAttachment(SceneRoot);
		EditorArrow->ArrowColor = FColor(255, 180, 0);
		EditorArrow->ArrowSize = 1.f;
		EditorArrow->bIsEditorOnly = true;
	}
#endif
}

bool AQuakeEnemySpawnPoint::IsEligibleForDifficulty(EQuakeDifficulty Current) const
{
	if (!bIsMarkedKillTarget)
	{
		return false;
	}
	return static_cast<uint8>(Current) >= static_cast<uint8>(MinDifficulty);
}

bool AQuakeEnemySpawnPoint::IsEligible() const
{
	EQuakeDifficulty Current = EQuakeDifficulty::Normal;
	if (const UWorld* World = GetWorld())
	{
		if (const AQuakeGameMode* GM = World->GetAuthGameMode<AQuakeGameMode>())
		{
			Current = GM->GetDifficulty();
		}
	}
	return IsEligibleForDifficulty(Current);
}

bool AQuakeEnemySpawnPoint::IsSatisfied() const
{
	return SpawnedEnemy != nullptr && SpawnedEnemy->IsDead();
}

void AQuakeEnemySpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	if (!bDeferredSpawn)
	{
		TrySpawn();
	}
}

void AQuakeEnemySpawnPoint::Activate(AActor* /*InInstigator*/)
{
	// Deferred spawn points ignore the instigator — the spawn location is
	// our own transform. Non-deferred points already spawned in BeginPlay;
	// the one-shot SpawnedEnemy guard makes this call a no-op for them.
	TrySpawn();
}

AQuakeEnemyBase* AQuakeEnemySpawnPoint::TrySpawn()
{
	if (SpawnedEnemy)
	{
		// One-shot: a spawn point spawns at most once per level attempt.
		return nullptr;
	}
	if (!IsEligible())
	{
		return nullptr;
	}
	if (!EnemyClass)
	{
		UE_LOG(LogQuakeSpawnPoint, Warning,
			TEXT("%s: EnemyClass is null — authoring error, fix in the editor."),
			*GetName());
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AQuakeEnemyBase* Enemy = World->SpawnActor<AQuakeEnemyBase>(
		EnemyClass, GetActorLocation(), GetActorRotation(), Params);
	if (Enemy)
	{
		// Stamp the marker flag so Die() routes the kill credit correctly
		// per SPEC 5.9. Directly-placed BP_Enemy_* actors keep the C++
		// default (false) and contribute nothing to KillsTotal.
		Enemy->bIsMarkedKillTarget = true;
		SpawnedEnemy = Enemy;
	}
	return Enemy;
}
