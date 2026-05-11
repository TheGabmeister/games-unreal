#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS(Abstract, meta = (PrioritizeCategories = "CB"))
class CB_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase();
	virtual void Tick(float DeltaTime) override;

	virtual void InitDirection(FVector Direction);

protected:
	UPROPERTY(VisibleAnywhere, Category = "CB")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "CB")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float Speed = 800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float Lifetime = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	float ArcGravity = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CB")
	TObjectPtr<USoundBase> LaunchSound;

	FVector Velocity;

	virtual void OnImpact(const FHitResult& Hit);

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
