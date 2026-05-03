// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GTASA : ModuleRules
{
	public GTASA(ReadOnlyTargetRules Target) : base(Target)
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
			"GTASA",
			"GTASA/Variant_Platforming",
			"GTASA/Variant_Platforming/Animation",
			"GTASA/Variant_Combat",
			"GTASA/Variant_Combat/AI",
			"GTASA/Variant_Combat/Animation",
			"GTASA/Variant_Combat/Gameplay",
			"GTASA/Variant_Combat/Interfaces",
			"GTASA/Variant_Combat/UI",
			"GTASA/Variant_SideScrolling",
			"GTASA/Variant_SideScrolling/AI",
			"GTASA/Variant_SideScrolling/Gameplay",
			"GTASA/Variant_SideScrolling/Interfaces",
			"GTASA/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
