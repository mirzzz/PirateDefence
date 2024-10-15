// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class NxPirateDefence : ModuleRules
{
	public NxPirateDefence(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // ����
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


        // ��� 
        PublicDependencyModuleNames.AddRange(new string[]
        {
            // ������Ʈ�� .h ���� ������ Module �߰�.
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
            // ������Ʈ�� .cpp ���� ������ Module �߰�.

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

            // ������Ʈ PlugIn�� �ڵ����� ���Եȴ�.
            "NxCodeBase",
            "NxCodeMode",
            "NxLoadingSplash",
            "NxLoadingUserWidget",
            //"NxDownloader"            
            "NxCodeUI",
        });

        // ���
        string kProjectPath = ModuleDirectory;                                       // ������Ʈ - VxVisualizer.Build.cs ���� ��ġ�� ����.
        string kEnginePath = System.IO.Path.GetFullPath(Target.RelativeEnginePath);  // ���� Root     - "UERoot/Engine/"
        string kSourceDeveloperPath = kEnginePath + "Source/Developer/";             // ���� Developer- "UERoot/Engine/Source/Developer/"
              

        /* �÷���
        [Target.Platform : UnrealTargetPlatform]
           Win64
           Max
           Linux
           Android
           IOS
           TVOS

       [Target.Configuration : UnrealTargetConfiguration]
            Debug
                - ���� �� ���� �ڵ� ��� ����� ������ ���·� ������.
                - Debug ȯ�漳���� ����ؼ� ������Ʈ�� �������ϰ�, �𸮾� �����ͷ� ������Ʈ�� ������ �ϴ� ���, 
                    "-debug" �÷��׸� �ٿ���� ������Ʈ�� �ڵ� ���泻���� �ݿ��Ǵ� ���� �� �� �ֽ��ϴ�.	

            DebugGame
                - ������ ����ȭ���� ����������, ���� �ڵ�� ����� ������ ���¸� �����Ѵ�. 
                - ������Ʈ�� ����� �ϱ⿡ ���� �̻����̴�.
                - Engine(Release), Game(Debug), ���� ���� ���(�ܼ� ���, ���, �������ϸ�) ��� ����

            Development
                - ���� �� ���� �ڵ带 ����ȭ �Ͽ� ������ �Ѵ�.
                - �����̳� �����ս��� �߿��� ��� �̻����̸�, �𸮾� �����ʹ� �⺻������ Development ȯ�漳���� ����մϴ�. 
                - ������Ʈ�� Development ȯ�漳������ �������ϸ� ������Ʈ�� ���� �ڵ� ��������� �����Ϳ� �ݿ����� Ȯ���� �� �ֽ��ϴ�. 
                - Engine(Release), Game(Release), ���� ���� ���(�ܼ� ���, ���, �������ϸ�) ��� ����

            Shipping
                - �ֻ��� �����ս��� ���� �߸ſ� ȯ�漳���Դϴ�. 
                - �ܼ� ���, ���, �������ϸ� ���� ���Ž�Ű�� ȯ�漳���Դϴ�.
                - Engine(Release), Game(Release), ���� ���� ��� X

            Test
                - ȯ�漳���� Shipping �� �� ���� �ܼ� ���, ���, �������ϸ� ����� �߰��� ���Դϴ�.
                - Engine(Release), Game(Release), ���� ���� �ּ� ��� ����.
        */

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // RTTI ���
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


        // ����Ƽ ���� �ý��� ��� ���� 
        //  - cpp file ���� ���� �ϳ��� ��� ������ �ӵ��� ������ �ϴ� ���� �ɼ� off ��.
        //  - IWYU Ȯ�� �ÿ��� ������
        bUseUnity = false;

        // Error::C4668 ����
        //bEnableUndefinedIdentifierWarnings = false;   

        // UE_LOG�� ���޵� ���ڿ� + �μ��� ���� ������ Ÿ�� ��ȿ�� �˻縦 Ȱ��ȭ
        // bValidateFormatStrings = true;

        // Exception On
        //bEnableExceptions = true;
    }
}
