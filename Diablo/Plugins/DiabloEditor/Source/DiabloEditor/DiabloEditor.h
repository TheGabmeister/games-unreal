#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDiabloEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
	void RegisterMenuExtension();

	static const FName TabId;
};
