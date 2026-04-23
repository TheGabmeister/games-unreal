// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Diablo : ModuleRules
{
	public Diablo(ReadOnlyTargetRules Target) : base(Target)
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
			"Diablo",
			"Diablo/Variant_Platforming",
			"Diablo/Variant_Platforming/Animation",
			"Diablo/Variant_Combat",
			"Diablo/Variant_Combat/AI",
			"Diablo/Variant_Combat/Animation",
			"Diablo/Variant_Combat/Gameplay",
			"Diablo/Variant_Combat/Interfaces",
			"Diablo/Variant_Combat/UI",
			"Diablo/Variant_SideScrolling",
			"Diablo/Variant_SideScrolling/AI",
			"Diablo/Variant_SideScrolling/Gameplay",
			"Diablo/Variant_SideScrolling/Interfaces",
			"Diablo/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
