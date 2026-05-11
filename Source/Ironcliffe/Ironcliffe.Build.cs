// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Ironcliffe : ModuleRules
{
	public Ironcliffe(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Ironcliffe",
			"Ironcliffe/Combat",
			"Ironcliffe/Systems",
			"Ironcliffe/UI",
			"Ironcliffe/Variant_Platforming",
			"Ironcliffe/Variant_Platforming/Animation",
			"Ironcliffe/Variant_Combat",
			"Ironcliffe/Variant_Combat/AI",
			"Ironcliffe/Variant_Combat/Animation",
			"Ironcliffe/Variant_Combat/Gameplay",
			"Ironcliffe/Variant_Combat/Interfaces",
			"Ironcliffe/Variant_Combat/UI",
			"Ironcliffe/Variant_SideScrolling",
			"Ironcliffe/Variant_SideScrolling/AI",
			"Ironcliffe/Variant_SideScrolling/Gameplay",
			"Ironcliffe/Variant_SideScrolling/Interfaces",
			"Ironcliffe/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

