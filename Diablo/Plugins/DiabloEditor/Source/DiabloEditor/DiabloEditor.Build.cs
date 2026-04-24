using UnrealBuildTool;

public class DiabloEditor : ModuleRules
{
	public DiabloEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UnrealEd",
			"Blutility",
			"Kismet",
			"UMG",
			"UMGEditor",
			"Slate",
			"SlateCore",
			"EnhancedInput",
			"NavigationSystem",
			"BSPUtils",
			"ToolMenus",
			"AssetTools",
			"Diablo"
		});

	}
}
