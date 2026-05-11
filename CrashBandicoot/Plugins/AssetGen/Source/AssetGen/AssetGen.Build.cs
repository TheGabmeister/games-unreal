using UnrealBuildTool;

public class AssetGen : ModuleRules
{
	public AssetGen(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"ToolMenus",
				"LevelEditor",
				"EnhancedInput",
				"InputCore",
				"UMG",
				"UMGEditor",
				"CB",
				"AssetTools",
			}
		);
	}
}
