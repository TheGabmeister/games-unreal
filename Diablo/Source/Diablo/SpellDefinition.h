#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpellDefinition.generated.h"

class ASpellProjectile;

UENUM(BlueprintType)
enum class ESpellEffect : uint8
{
	Projectile,
	Heal,
	AoE,
	TownPortal,
	Teleport,
	Debuff,
	Buff
};

UCLASS(BlueprintType)
class DIABLO_API USpellDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spell")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spell")
	float ManaCost = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spell")
	float Cooldown = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spell")
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spell")
	TSubclassOf<ASpellProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spell")
	ESpellEffect Effect = ESpellEffect::Projectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spell")
	float HealAmount = 0.f;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("SpellDefinition", GetFName());
	}
};
