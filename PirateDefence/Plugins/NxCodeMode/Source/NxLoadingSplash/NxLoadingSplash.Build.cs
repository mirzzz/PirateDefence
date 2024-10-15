// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

// Layout 형태의 로딩 지원
//	- 시작시 Splash 형태 로딩 지원
//	- 다양한 Layout 형태 로딩 지원
public class NxLoadingSplash : ModuleRules
{
	public NxLoadingSplash(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				ModuleDirectory,
                Path.Combine(ModuleDirectory, "Function"),
                Path.Combine(ModuleDirectory, "Widget"),
            }
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				ModuleDirectory,
                //Path.Combine(ModuleDirectory, "Private"),
            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
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

				"MoviePlayer",
                "DeveloperSettings",
                "InputCore",
                "PreLoadScreen",
                "RenderCore",
                "Projects",			// IPluginManager                

				"NxCodeBase",
            }
			);		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        // 유니티 빌드 시스템 기능 끄기 
        //  - cpp file 여러 개를 하나로 묶어서 컴파일 속도를 빠르게 하는 빌드 옵션 off 함.
        //  - IWYU 확인 시에도 유용함
        bUseUnity = false;
    }
}
