#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaunchedEnemyProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS(meta = (PrioritizeCategories = "CB"))
class CB_API ALaunchedEnemyProjectile : public AActor
{
	GENERATED_BODY()

public:
	ALaunchedEnemyProjectile();
	virtual void Tick(float DeltaTime) override;

	void InitProjectile(FVector Direction, float Speed);

protected:
	UPROPERTY(VisibleAnywhere, Category = "CB")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "CB")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float Lifetime = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float GravityScale = 1.0f;

	FVector Velocity;

	UFUNCTION()
	void OnProjectileOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
