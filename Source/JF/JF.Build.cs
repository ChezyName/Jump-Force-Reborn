// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class JF : ModuleRules
{
	public JF(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "TakeRecorder" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", 
			"EnhancedInput", "GameplayAbilities", "GameplayTags", "GameplayTasks",
			"Niagara", "UMG"
		});
	}
}
