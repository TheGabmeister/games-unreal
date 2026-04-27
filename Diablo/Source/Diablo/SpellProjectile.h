#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpellProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS(Abstract)
class DIABLO_API ASpellProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASpellProjectile();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spell")
	float ManaCost = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spell")
	float Cooldown = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spell")
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spell")
	float Speed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell")
	bool bDamageEnemies = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell")
	bool bDamageHero = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spell")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<AController> InstigatorController;
};
