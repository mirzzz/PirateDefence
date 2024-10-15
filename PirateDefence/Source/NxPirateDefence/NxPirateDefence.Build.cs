// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class NxPirateDefence : ModuleRules
{
	public NxPirateDefence(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 파일
        PublicIncludePaths.AddRange(new string[]
        {
            ModuleDirectory,
            //Path.Combine(ModuleDirectory, "Folder"),
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            ModuleDirectory,
            //Path.Combine(ModuleDirectory, "Folder"),
        });


        // 모듈 
        PublicDependencyModuleNames.AddRange(new string[]
        {
            // 프로젝트의 .h 에서 접근할 Module 추가.
			"Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "Hotfix",
            "Sockets",
            "Networking"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            // 프로젝트의 .cpp 에서 접근할 Module 추가.

            "Slate",
            "SlateCore",
            "RenderCore",
            "DeveloperSettings",
            "ApplicationCore",      // platformInputDevice
            "ProceduralMeshComponent",

            "Landscape",            // landscape support
            "Foliage",
            "CommonInput",
            "CommonUI",
            "RHI",
            "Projects",             // IPluginManager
                        
            "GameplayTags",         // GameAbilitySystem
            "GameplayTasks",            
            "NetCore",
            "Niagara",

            "AudioModulation",
            "AudioMixer",
            "MediaAssets",

            "EnhancedInput",        // Input
            "CommonInput",
            "CommonUI",

            // 프로젝트 PlugIn은 자동으로 포함된다.
            "NxCodeBase",
            "NxCodeMode",
            "NxLoadingSplash",
            "NxLoadingUserWidget",
            //"NxDownloader"            
            "NxCodeUI",
        });

        // 경로
        string kProjectPath = ModuleDirectory;                                       // 프로젝트 - VxVisualizer.Build.cs 파일 위치가 기준.
        string kEnginePath = System.IO.Path.GetFullPath(Target.RelativeEnginePath);  // 엔진 Root     - "UERoot/Engine/"
        string kSourceDeveloperPath = kEnginePath + "Source/Developer/";             // 엔진 Developer- "UERoot/Engine/Source/Developer/"
              

        /* 플랫폼
        [Target.Platform : UnrealTargetPlatform]
           Win64
           Max
           Linux
           Android
           IOS
           TVOS

       [Target.Configuration : UnrealTargetConfiguration]
            Debug
                - 엔진 및 게임 코드 모두 디버깅 가능한 상태로 컴파일.
                - Debug 환경설정을 사용해서 프로젝트를 컴파일하고, 언리얼 에디터로 프로젝트를 열고자 하는 경우, 
                    "-debug" 플래그를 붙여줘야 프로젝트에 코드 변경내용이 반영되는 것을 볼 수 있습니다.	

            DebugGame
                - 엔진은 최적화시켜 빌드하지만, 게임 코드는 디버깅 가능한 상태를 유지한다. 
                - 프로젝트를 디버깅 하기에 가장 이상적이다.
                - Engine(Release), Game(Debug), 개발 도움 기능(콘솔 명령, 통계, 프로파일링) 사용 가능

            Development
                - 엔진 및 게임 코드를 최적화 하여 컴파일 한다.
                - 개발이나 퍼포먼스가 중요한 경우 이상적이며, 언리얼 에디터는 기본적으로 Development 환경설정을 사용합니다. 
                - 프로젝트를 Development 환경설정으로 컴파일하면 프로젝트에 가한 코드 변경사항을 에디터에 반영시켜 확인할 수 있습니다. 
                - Engine(Release), Game(Release), 개발 도움 기능(콘솔 명령, 통계, 프로파일링) 사용 가능

            Shipping
                - 최상의 퍼포먼스와 게임 발매용 환경설정입니다. 
                - 콘솔 명령, 통계, 프로파일링 툴을 제거시키는 환경설정입니다.
                - Engine(Release), Game(Release), 개발 도움 기능 X

            Test
                - 환경설정은 Shipping 에 몇 가지 콘솔 명령, 통계, 프로파일링 기능이 추가된 것입니다.
                - Engine(Release), Game(Release), 개발 도움 최소 기능 포함.
        */

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // RTTI 사용
            bUseRTTI = true;

            switch (Target.Configuration)
            {
                case UnrealTargetConfiguration.Debug:
                case UnrealTargetConfiguration.DebugGame:
                case UnrealTargetConfiguration.Development:
                    break;

                case UnrealTargetConfiguration.Test:
                case UnrealTargetConfiguration.Shipping:
                    if (Target.bBuildDeveloperTools)
                    {
                        PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=1");
                    }
                    break;
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Android) { }
        else if (Target.Platform == UnrealTargetPlatform.IOS) { }

        // Game/Editor
        switch (Target.Type)
        {
            case TargetType.Game:
                break;

            case TargetType.Editor:
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
                break;
        }


        // 유니티 빌드 시스템 기능 끄기 
        //  - cpp file 여러 개를 하나로 묶어서 컴파일 속도를 빠르게 하는 빌드 옵션 off 함.
        //  - IWYU 확인 시에도 유용함
        bUseUnity = false;

        // Error::C4668 무시
        //bEnableUndefinedIdentifierWarnings = false;   

        // UE_LOG에 전달된 문자열 + 인수에 대한 컴파일 타임 유효성 검사를 활성화
        // bValidateFormatStrings = true;

        // Exception On
        //bEnableExceptions = true;
    }
}
