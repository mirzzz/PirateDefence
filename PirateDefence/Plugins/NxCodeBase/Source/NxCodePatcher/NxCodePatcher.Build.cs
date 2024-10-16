// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
public class NxCodePatcher : ModuleRules
{
	public NxCodePatcher(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				ModuleDirectory,
				Path.Combine(ModuleDirectory, "Download"),
                Path.Combine(ModuleDirectory, "Patch"),
				// ... add public include paths required here ...
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				ModuleDirectory,
				//Path.Combine(ModuleDirectory, "Download"),
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
				"ApplicationCore",
				"Json",

                "NxCodeBase",
            }
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
				"HTTP",
				"PakFile",
				"DeveloperSettings",    // UDeveloperSettings
				"Projects",				// IPluginManager
				"UMG",
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			switch (Target.Configuration)
			{
				case UnrealTargetConfiguration.Debug:
				case UnrealTargetConfiguration.DebugGame:
				case UnrealTargetConfiguration.Development:
					break;

				case UnrealTargetConfiguration.Test:
				case UnrealTargetConfiguration.Shipping:
					break;
			}

			switch (Target.Type)
			{
				case TargetType.Game:
					PrivateDependencyModuleNames.AddRange(new string[] { });
					break;

				case TargetType.Editor:
					PrivateDependencyModuleNames.AddRange(new string[] { });
					break;
			}
			 
		}
		else if (Target.Platform == UnrealTargetPlatform.Android) { }
		else if (Target.Platform == UnrealTargetPlatform.IOS) { }

        // 유니티 빌드 시스템 기능 끄기 
        //  - cpp file 여러 개를 하나로 묶어서 컴파일 속도를 빠르게 하는 빌드 옵션 off 함.
        //  - IWYU 확인 시에도 유용함
        bUseUnity = false;

    }
}
