// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ExpJam26 : ModuleRules
{
	public ExpJam26(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"ExpJam26",
			"ExpJam26/Inventory",
			"ExpJam26/Inventory/UI",
			"ExpJam26/Variant_Horror",
			"ExpJam26/Variant_Horror/UI",
			"ExpJam26/Variant_Shooter",
			"ExpJam26/Variant_Shooter/AI",
			"ExpJam26/Variant_Shooter/UI",
			"ExpJam26/Variant_Shooter/Weapons",
			"ExpJam26/Customer"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
