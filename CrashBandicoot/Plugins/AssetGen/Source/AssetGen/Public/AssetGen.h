#pragma once

#include "Modules/ModuleManager.h"

class FAssetGenModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();
	TSharedRef<class SDockTab> OnSpawnTab(const class FSpawnTabArgs& SpawnTabArgs);

	static FReply OnCreateGameModeClicked();
	static FReply OnCreateGameInstanceClicked();
	static FReply OnCreatePlayerCharacterClicked();
	static FReply OnCreatePlayerControllerClicked();
	static FReply OnCreateDebugLevelClicked();
	static FReply OnCreateInputActionsClicked();
	static FReply OnCreateIMCClicked();
	static FReply OnCreateCameraClicked();
	static FReply OnCreateSpinMontageClicked();

	// Phase 2 buttons
	static FReply OnCreateCratesClicked();
	static FReply OnCreatePickupsClicked();
	static FReply OnCreateAkuAkuMaskClicked();
	static FReply OnCreateGameplayHUDClicked();
	static FReply OnCreateLoadingScreenClicked();
};
