#include "Pickup.h"

#include "CBEvents.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameplayMessageSubsystem.h"

// Sets default values
APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	OnActorBeginOverlap.AddDynamic(this, &APickup::OnPickup);
}

// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickup::OnPickup_Implementation(AActor* OverlappedActor, AActor* OtherActor)
{
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}

	//FGameplayMessageInt Message;
	//Message.Value = 1;
	//UGameplayMessageSubsystem::Get(this).BroadcastMessage(TriggerEventChannelOnCollision, Message);

	//Destroy();
}

