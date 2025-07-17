#include "Pickup.h"

#include "CBGameplayTags.h"
#include "CBStructs.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayTagContainer.h"

// Sets default values
APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	GetStaticMeshComponent()->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnMeshBeginOverlap);
}

// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickup::OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
								 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
								 bool bFromSweep, const FHitResult& SweepResult)
{
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}
	
	FGameplayMessageInt Message;
	Message.Value = 10;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(Events_OnPickedUp_WumpaFruit, Message);

	
	Destroy();
	
}

