// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ImpactBeyond : ModuleRules
{
	public ImpactBeyond(ReadOnlyTargetRules Target) : base(Target)
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

		PrivateDependencyModuleNames.Add("AnimationCore");

		PublicIncludePaths.AddRange(new string[] {
			"ImpactBeyond",
			"ImpactBeyond/Variant_Platforming",
			"ImpactBeyond/Variant_Platforming/Animation",
			"ImpactBeyond/Variant_Combat",
			"ImpactBeyond/Variant_Combat/AI",
			"ImpactBeyond/Variant_Combat/Animation",
			"ImpactBeyond/Variant_Combat/Gameplay",
			"ImpactBeyond/Variant_Combat/Interfaces",
			"ImpactBeyond/Variant_Combat/UI",
			"ImpactBeyond/Variant_SideScrolling",
			"ImpactBeyond/Variant_SideScrolling/AI",
			"ImpactBeyond/Variant_SideScrolling/Gameplay",
			"ImpactBeyond/Variant_SideScrolling/Interfaces",
			"ImpactBeyond/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
