// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class NxCodeBase : ModuleRules
{
	public NxCodeBase(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // 다른 모듈에 노출되는 파일을 포함할 모든 경로 목록
        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				ModuleDirectory,
				// 외부 모듈에서 경로를 넣지 않고 헤더 파일을 바로 Include 하려면 경로 추가 해줌.
                Path.Combine(ModuleDirectory, "Frame"),
                Path.Combine(ModuleDirectory, "Utility"),
                Path.Combine(ModuleDirectory, "Function"),
                Path.Combine(ModuleDirectory, "Table"),
                Path.Combine(ModuleDirectory, "Settings"),
                Path.Combine(ModuleDirectory, "Network"),
                Path.Combine(ModuleDirectory, "Function"),                
            }
			);

        // 다른 모듈에 노출되지 않는 현재 모듈의 내부에만 포함할 파일에 대한 모든 경로 목록
        PrivateIncludePaths.AddRange(
			new string[] {
                ModuleDirectory,
				// ... add other private include paths required here ...
			}
			);


        // Public 종속성 모듈 이름 목록(경로 필요 없음)(Public/Private 포함이 자동으로 수행됨).
        //	- 이들은 Public 소스 파일에 필요한 모듈입니다.
        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"XmlParser",        // support xml
                "Json",             // support json

                // ... add other public dependencies that you statically link with here ...
                // 종속성 
                "NxCodeBasePrev",			}
			);


        // Private 종속성 모듈 이름 목록입니다.
        //	- Private 코드가 의존하는 모듈이지만 Public Include 파일에서는 아무것도 의존하지 않습니다.
        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	

				"GeometryCore",         // FDynamicMesh
                "DynamicMesh",          // GeometryProcessing
                "ProceduralMeshComponent",
                "DeveloperSettings",    // UDeveloperSettings
				"ApplicationCore",      // IPlatformInputDevice
				"Projects",             // IPluginManager
                "HTTP",

                "Foliage",				// AFoliage
                "Landscape",            // ALandscape
			}
			);


        // 모듈의 Public 헤더가 액세스해야 하는 헤더 파일이 포함된 모듈 이름(경로는 필요하지 않음)의 목록이지만 가져오거나 연결할 필요는 없습니다.
        PublicIncludePathModuleNames.AddRange(
            new string[] {

            });

        // 모듈의 Privae 헤더가 액세스해야 하는 헤더 파일이 포함된 모듈 이름(경로는 필요하지 않음)의 목록이지만 가져오거나 연결할 필요는 없습니다.
        PrivateIncludePathModuleNames.AddRange(
            new string[] {

            });

        // 런타임에 필요할 수 있는 추가 모듈
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );

        // Editor 빌드에만 포함되기를 원할 경우.
        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "UnrealEd",
                    "LandscapeEditor",
                    "FoliageEdit",
                    "LevelEditor",
                }
            );
        }

        // 유니티 빌드 시스템 기능 끄기 
        //  - cpp file 여러 개를 하나로 묶어서 컴파일 속도를 빠르게 하는 빌드 옵션 off 함.
        //  - IWYU 확인 시에도 유용함
        bUseUnity = false;
    }
}
