// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseActor2D.h"
#include "Projectile.generated.h"

UCLASS(abstract)
class GALAXIAN_API AProjectile : public ABaseActor2D
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Projectile speed, editable in the editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FVector Velocity{ 0.f, 0.f, 1000.0f };
};
