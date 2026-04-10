#include "QuakeTargetDummy.h"

#include "QuakeCollisionChannels.h"

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/CollisionProfile.h"
#include "UObject/ConstructorHelpers.h"

AQuakeTargetDummy::AQuakeTargetDummy()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	// Pull the engine cube as a sensible default so the actor renders
	// without a BP subclass having to fill in a mesh slot. Phase 2 is
	// editor-only test scaffolding; the cube doesn't need to be tunable.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	// Block everything by default, including the Weapon trace channel so
	// the Axe can hit it. The dummy is treated as a WorldDynamic so the
	// stock BlockAll profile already covers most channels — we just have
	// to make sure the custom Weapon trace lands on Block, which it does
	// via the channel's DefaultResponse=ECR_Block in DefaultEngine.ini.
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	Mesh->SetCollisionResponseToChannel(QuakeCollision::ECC_Weapon, ECR_Block);

	HealthText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HealthText"));
	HealthText->SetupAttachment(Mesh);
	HealthText->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	HealthText->SetHorizontalAlignment(EHTA_Center);
	HealthText->SetVerticalAlignment(EVRTA_TextCenter);
	HealthText->SetTextRenderColor(FColor::White);
	HealthText->SetWorldSize(40.f);
}

void AQuakeTargetDummy::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	UpdateHealthText();
}

float AQuakeTargetDummy::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	if (Health <= 0.f)
	{
		return 0.f;  // Already dead — short-circuit before the engine pipeline.
	}

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health = FMath::Max(0.f, Health - ActualDamage);
	UpdateHealthText();

	if (Health <= 0.f)
	{
		HealthText->SetText(FText::FromString(TEXT("DEAD")));
		HealthText->SetTextRenderColor(FColor::Red);
		SetCanBeDamaged(false);
	}

	return ActualDamage;
}

void AQuakeTargetDummy::UpdateHealthText()
{
	if (HealthText)
	{
		HealthText->SetText(FText::FromString(FString::Printf(TEXT("HP: %d"), FMath::RoundToInt(Health))));
	}
}
