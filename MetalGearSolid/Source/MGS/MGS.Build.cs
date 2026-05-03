// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MGS : ModuleRules
{
	public MGS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MGS",
			"MGS/Variant_Platforming",
			"MGS/Variant_Platforming/Animation",
			"MGS/Variant_Combat",
			"MGS/Variant_Combat/AI",
			"MGS/Variant_Combat/Animation",
			"MGS/Variant_Combat/Gameplay",
			"MGS/Variant_Combat/Interfaces",
			"MGS/Variant_Combat/UI",
			"MGS/Variant_SideScrolling",
			"MGS/Variant_SideScrolling/AI",
			"MGS/Variant_SideScrolling/Gameplay",
			"MGS/Variant_SideScrolling/Interfaces",
			"MGS/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
