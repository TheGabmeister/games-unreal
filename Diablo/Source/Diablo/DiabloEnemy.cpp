#include "DiabloEnemy.h"
#include "Diablo.h"
#include "Components/CapsuleComponent.h"

ADiabloEnemy::ADiabloEnemy()
{
	CurrentHP = MaxHP;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 90.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

float ADiabloEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHP = FMath::Max(0.f, CurrentHP - ActualDamage);
	UE_LOG(LogDiablo, Display, TEXT("%s took %.0f damage (HP: %.0f/%.0f)"),
		*GetName(), ActualDamage, CurrentHP, MaxHP);

	if (CurrentHP <= 0.f)
	{
		UE_LOG(LogDiablo, Display, TEXT("%s died"), *GetName());
		Destroy();
	}

	return ActualDamage;
}
