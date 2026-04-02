// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using EpicGames.Core;
using UnrealBuildTool;

public class RoadManager : ModuleRules
{
	public RoadManager(ReadOnlyTargetRules Target) : base(Target)
	{
		string RoadManagerDir = Path.Combine(ModuleDirectory, "modules", "esmini");
		string RoadManagerSrcDir = Path.Combine(RoadManagerDir, "EnvironmentSimulator", "Modules");
		string RoadManagerCppDir = Path.Combine(RoadManagerSrcDir, "RoadManager");
		string CommonMiniCppDir = Path.Combine(RoadManagerSrcDir, "CommonMini");
		string PugixmlDir = Path.Combine(RoadManagerDir, "externals", "pugixml");
		string YamlDir = Path.Combine(RoadManagerDir, "externals", "yaml");

		PublicIncludePaths.AddRange(
			new string[] {
				RoadManagerCppDir,
				CommonMiniCppDir,
				PugixmlDir,
				YamlDir,
				Path.Combine(RoadManagerDir, "externals", "fmt", "include"),
			}
		);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			Type = ModuleType.External;

			PublicDefinitions.Add("__MINGW32__=0");
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "RelWithDebInfo", "RoadManager.lib"));
			PublicDelayLoadDLLs.Add("RoadManager.dll");
			RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "bin", "RelWithDebInfo", "RoadManager.dll"));
		}
		else
		{
			// Non-Windows: link esmini RoadManager + CommonMini + pugixml from source. The default module root
			// contains a .ubtignore so UBT does not compile the full esmini tree; only the directories below are built.
			Type = ModuleType.CPlusPlus;

			PublicDefinitions.Add("__MINGW32__=0");
			PublicDefinitions.Add("FMT_HEADER_ONLY=1");

			bEnableExceptions = true;
			bUseUnity = false;
			bDisableStaticAnalysis = true;
			PCHUsage = PCHUsageMode.NoPCHs;
			IWYUSupport = IWYUSupport.None;

			ConditionalAddModuleDirectory(new DirectoryReference(Path.Combine(ModuleDirectory, "Stubs")));
			ConditionalAddModuleDirectory(new DirectoryReference(RoadManagerCppDir));
			ConditionalAddModuleDirectory(new DirectoryReference(CommonMiniCppDir));
			ConditionalAddModuleDirectory(new DirectoryReference(PugixmlDir));
			ConditionalAddModuleDirectory(new DirectoryReference(YamlDir));
		}
	}
}
