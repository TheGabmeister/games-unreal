#pragma once

#include "CoreMinimal.h"
#include "Enemy/CBEnemyCharacterBase.h"
#include "JumpingEnemy.generated.h"

UENUM(BlueprintType)
enum class EJumpingEnemyState : uint8 { Grounded, Jumping, Dead };

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API AEnemyJumping : public ACBEnemyCharacterBase
{
	GENERATED_BODY()

public:
	AEnemyJumping();
	virtual void Tick(float DeltaTime) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnJumpHit(ACBPlayerCharacter* Player) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "CB")
	EJumpingEnemyState CurrentState = EJumpingEnemyState::Grounded;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float GroundedDuration = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float JumpImpulseZ = 800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float JumpImpulseForward = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	bool bExtraHighBounce = false;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float ExtraHighBounceVelocity = 1400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	bool bDangerousWhileAirborne = false;

	float StateTimer = 0.0f;

	virtual void HandleDefeat() override;
	virtual void BeginPlay() override;
};
