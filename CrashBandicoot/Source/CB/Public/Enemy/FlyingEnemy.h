#pragma once

#include "CoreMinimal.h"
#include "Enemy/CBEnemyCharacterBase.h"
#include "FlyingEnemy.generated.h"

UENUM(BlueprintType)
enum class EFlyingEnemyState : uint8 { Flying, Perched, Swooping, Returning, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyFlying : public ACBEnemyCharacterBase
{
	GENERATED_BODY()

public:
	AEnemyFlying();
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "CB")
	EFlyingEnemyState CurrentState = EFlyingEnemyState::Flying;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	bool bIsSwooper = false;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float FlySpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float SwoopDepth = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float SwoopDuration = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float ReturnDuration = 1.5f;

	float CurrentSplineDistance = 0.0f;
	FVector PerchLocation;
	float StateAlpha = 0.0f;

	virtual void BeginPlay() override;

	virtual void HandleDefeat() override;
	void TickFlying(float DeltaTime);
	void TickSwooping(float DeltaTime);
	void TickReturning(float DeltaTime);
};
