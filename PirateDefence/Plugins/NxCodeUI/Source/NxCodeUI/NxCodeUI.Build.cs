// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class NxCodeUI : ModuleRules
{
	public NxCodeUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
                ModuleDirectory,
				// ... add public include paths required here ...
                Path.Combine(ModuleDirectory, "Frame"),
                Path.Combine(ModuleDirectory, "Function"),
                Path.Combine(ModuleDirectory, "Table"),
                Path.Combine(ModuleDirectory, "Widget"),
                Path.Combine(ModuleDirectory, "Widget/Dialog"),
            }
			); 
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
                ModuleDirectory,
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Engine",
				// ... add other public dependencies that you statically link with here ...
                "InputCore",
                "Slate",

                "CommonInput",
                "CommonUI",
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
				"ApplicationCore",
                "UMG",
                "WidgetCarousel",
                "MediaAssets",
                "AudioMixer",
                "MoviePlayer",
                "WebBrowser",
                "GameplayTags",
                "DeveloperSettings",

                "NxCodeBase",
                "NxCodeWindows",
            }
			);

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
                // "Analytics",
                // "AnalyticsET",
                // "EngineSettings",
                // "AudioMixer",
                // "DeveloperSettings"
            }
            );

        // 유니티 빌드 시스템 기능 끄기 
        //  - cpp file 여러 개를 하나로 묶어서 컴파일 속도를 빠르게 하는 빌드 옵션 off 함.
        //  - IWYU 확인 시에도 유용함
        bUseUnity = false;
    }
}
