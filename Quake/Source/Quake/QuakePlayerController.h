#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "QuakePlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class QUAKE_API AQuakePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	/**
	 * Weapon-swap inputs. IA_Weapon1 → Axe (slot 1), IA_Weapon2 → Shotgun
	 * (slot 2), IA_Weapon4 → Nailgun (slot 4), IA_Weapon7 → Rocket Launcher
	 * (slot 7). Authored in the editor, mapped to keyboard 1 / 2 / 4 / 7
	 * in IMC_Default, and assigned via BP_QuakePlayerController defaults.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Weapons")
	TObjectPtr<UInputAction> Weapon1Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Weapons")
	TObjectPtr<UInputAction> Weapon2Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Weapons")
	TObjectPtr<UInputAction> Weapon4Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Weapons")
	TObjectPtr<UInputAction> Weapon7Action;

protected:
	virtual void BeginPlay() override;
};
