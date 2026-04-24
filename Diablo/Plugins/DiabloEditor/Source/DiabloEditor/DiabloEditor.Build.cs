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
			"Slate",
			"SlateCore",
			"EnhancedInput",
			"Diablo"
		});

	}
}
