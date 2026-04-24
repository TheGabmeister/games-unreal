#include "DiabloEditor.h"
#include "SDiabloEditorToolPanel.h"
#include "Modules/ModuleManager.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"

const FName FDiabloEditorModule::TabId(TEXT("DiabloEditorTools"));

void FDiabloEditorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		TabId,
		FOnSpawnTab::CreateRaw(this, &FDiabloEditorModule::SpawnTab))
		.SetDisplayName(FText::FromString(TEXT("Diablo Tools")))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	RegisterMenuExtension();
}

void FDiabloEditorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabId);
}

TSharedRef<SDockTab> FDiabloEditorModule::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SDiabloEditorToolPanel)
		];
}

void FDiabloEditorModule::RegisterMenuExtension()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		FToolMenuSection& Section = Menu->FindOrAddSection("DiabloTools");
		Section.Label = FText::FromString(TEXT("Diablo"));

		Section.AddMenuEntry(
			"OpenDiabloTools",
			FText::FromString(TEXT("Diablo Tools")),
			FText::FromString(TEXT("Open the Diablo asset generator panel")),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(TabId);
			}))
		);
	}));
}

IMPLEMENT_MODULE(FDiabloEditorModule, DiabloEditor)
