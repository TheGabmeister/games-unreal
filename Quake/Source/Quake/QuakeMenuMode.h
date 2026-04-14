#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "Templates/SharedPointer.h"
#include "QuakeMenuMode.generated.h"

class SQuakeMainMenu;

/**
 * Phase 13 main-menu HUD. Used by editor-authored MainMenu.umap. Hosts
 * the SQuakeMainMenu Slate widget on BeginPlay and tears it down on
 * EndPlay. Set the HUDClass on AQuakeMenuGameMode to point at this.
 */
UCLASS()
class QUAKE_API AQuakeMenuHUD : public AHUD
{
	GENERATED_BODY()

public:
	/** Hub map opened by "New Game" after difficulty selection. Set on BP_QuakeMenuHUD. */
	UPROPERTY(EditDefaultsOnly, Category = "Menu")
	FName HubMapName;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	TSharedPtr<SQuakeMainMenu> MenuWidget;
};

/**
 * Phase 13 main-menu GameMode. Identical to the default GameMode except
 * for HUDClass = AQuakeMenuHUD. Assigned as MainMenu.umap's GameMode in
 * the World Settings.
 */
UCLASS()
class QUAKE_API AQuakeMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AQuakeMenuGameMode();
};
