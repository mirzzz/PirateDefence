// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

using System.IO;
using System.Collections.Generic;

public class NxCodeModeEditor : ModuleRules
{
	public NxCodeModeEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
                // ... add public include paths required here ...
				ModuleDirectory,
				// 외부 모듈에서 경로를 넣지 않고 헤더 파일을 바로 Include 하려면 경로 추가 해줌.
                //Path.Combine(ModuleDirectory, "Widgets"),
            }
            );
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
                // ... add other private include paths required here ...
                ModuleDirectory,	
                // 외부 모듈에서 경로를 넣고 헤더 파일을 Include 하려면 경로 추가 해줌.
                //Path.Combine(ModuleDirectory, "Widgets"),
            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Engine",
                "InputCore",

                "UnrealEd",                
                "MeshDescription",
                "StaticMeshDescription",
                "WebBrowser",                
                "Json",
                "JsonUtilities",
                "BSPUtils",
				// ... add other public dependencies that you statically link with here ...

                "NxCodeBase",
			});

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "AssetRegistry",
                "AssetTools",
                "ContentBrowser",
                "Landscape"
            });

        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "CoreUObject",
                "Slate",
                "SlateCore",
                "Engine",
                "EditorFramework",
                "UnrealEd",
                "ContentBrowserData",
                "Projects",
				"InputCore",
				"ToolMenus",
                "EditorStyle",
                "UMG",
                "Landscape",
                "LevelEditor",
                "PropertyEditor",
                "DesktopPlatform",
                "MainFrame",
                "SourceControl",
                "SourceControlWindows",
                "NewLevelDialog",
                "LandscapeEditor",
                "FoliageEdit",
                "Foliage",
                "MaterialUtilities",
                "EditorWidgets",
                "Blutility",
                "EditorSubsystem",
                
                "DeveloperToolSettings",
                "MeshMergeUtilities",
                "DeveloperSettings",                    // UDeveloperSettings
                "StatusBar",                            // UStatusBarSubsystem
                "EditorConfig",                         // UEditorConfigBase
                "ComponentVisualizers",                 // FComponentVisualizer

				// ... add private dependencies that you statically link with here ...	
                
            }
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
                "AssetRegistry",
                "AssetTools",
                "SceneOutliner",
                "ContentBrowser"
				// ... add any modules that your module loads dynamically here ...
			});


        // 유니티 빌드 시스템 기능 끄기 
        //  - cpp file 여러 개를 하나로 묶어서 컴파일 속도를 빠르게 하는 빌드 옵션 off 함.
        //  - IWYU 확인 시에도 유용함
        bUseUnity = false;
    }
}
