#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

UENUM(BlueprintType)
enum class EEquipSlot : uint8
{
	None,
	Head,
	Chest,
	LeftHand,
	RightHand,
	LeftRing,
	RightRing,
	Amulet
};

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Misc,
	Weapon,
	Armor,
	Shield,
	Helm,
	Ring,
	Amulet,
	Potion,
	Scroll,
	Gold
};

UCLASS(BlueprintType)
class DIABLO_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Grid")
	int32 GridWidth = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Grid")
	int32 GridHeight = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bStackable = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 MaxStack = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EEquipSlot EquipSlot = EEquipSlot::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EItemCategory Category = EItemCategory::Misc;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stats")
	float BonusStr = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stats")
	float BonusMag = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stats")
	float BonusDex = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stats")
	float BonusVit = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stats")
	float MinDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stats")
	float MaxDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stats")
	float ArmorClass = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stats")
	int32 MaxDurability = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 GoldValue = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	float HealAmount = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 QualityLevel = 1;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("ItemDefinition", GetFName());
	}
};
