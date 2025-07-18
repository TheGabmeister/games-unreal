#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "GameplayTagContainer.h"
#include "Pickup.generated.h"

struct FGameplayTag;

UCLASS()
class CRASHBANDICOOT_API APickup : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Default", meta = (AllowPrivateAccess = "true"))
	USoundWave* PickupSound;

	UPROPERTY(EditAnywhere, Category = "Default", meta = (AllowPrivateAccess = "true"))
	FGameplayTag TriggerEventChannelOnCollision;
	
	UFUNCTION()
	void OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
							bool bFromSweep, const FHitResult& SweepResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};