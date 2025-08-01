// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CrashBandicoot : ModuleRules
{
	public CrashBandicoot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"UMG", 
			"GameplayTags",
			"GameplayMessageRuntime"
		});
	}
}
