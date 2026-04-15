// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FF7 : ModuleRules
{
	public FF7(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"Slate",
			"SlateCore",
			"UMG",
			"GameplayTags",
			"DeveloperSettings",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// Flat module layout: expose the module root so files in subfolders (e.g. Tests/) can include siblings without "../" paths.
		PublicIncludePaths.Add(ModuleDirectory);

		if (Target.bBuildDeveloperTools || (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Configuration != UnrealTargetConfiguration.Test))
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"AutomationController",
				"FunctionalTesting",
			});
		}
	}
}
