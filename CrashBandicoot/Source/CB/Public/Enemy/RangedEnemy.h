#pragma once

#include "CoreMinimal.h"
#include "Enemy/CBEnemyCharacterBase.h"
#include "RangedEnemy.generated.h"

class AProjectileBase;

UENUM(BlueprintType)
enum class ERangedEnemyState : uint8 { Idle, Throwing, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyRanged : public ACBEnemyCharacterBase
{
	GENERATED_BODY()

public:
	AEnemyRanged();
	virtual void Tick(float DeltaTime) override;

protected:
	ERangedEnemyState CurrentState = ERangedEnemyState::Idle;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TSubclassOf<AProjectileBase> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float ThrowInterval = 2.5f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float ThrowWindupTime = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	bool bAimAtPlayer = false;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	FVector ProjectileSpawnOffset = FVector(50.f, 0.f, 50.f);

	void SpawnProjectile();
	float StateTimer = 0.0f;

	virtual void HandleDefeat() override;
	virtual void BeginPlay() override;
};
