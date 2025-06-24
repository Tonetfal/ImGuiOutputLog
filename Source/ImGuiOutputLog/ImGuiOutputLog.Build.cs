// Author: Antonio Sidenko (Tonetfal), June 2025

using System.IO;
using UnrealBuildTool;

public class ImGuiOutputLog : ModuleRules
{
	public ImGuiOutputLog(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.Add(Path.Combine(Path.GetFullPath(Path.Combine(ModuleDirectory)), "Private"));
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"DeveloperSettings",
				"Engine",
				"ImGui", 
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
