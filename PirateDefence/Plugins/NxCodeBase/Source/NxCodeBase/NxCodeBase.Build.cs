// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class NxCodeBase : ModuleRules
{
	public NxCodeBase(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // �ٸ� ��⿡ ����Ǵ� ������ ������ ��� ��� ���
        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				ModuleDirectory,
				// �ܺ� ��⿡�� ��θ� ���� �ʰ� ��� ������ �ٷ� Include �Ϸ��� ��� �߰� ����.
                Path.Combine(ModuleDirectory, "Frame"),
                Path.Combine(ModuleDirectory, "Utility"),
                Path.Combine(ModuleDirectory, "Function"),
                Path.Combine(ModuleDirectory, "Table"),
                Path.Combine(ModuleDirectory, "Settings"),
                Path.Combine(ModuleDirectory, "Network"),
                Path.Combine(ModuleDirectory, "Function"),                
            }
			);

        // �ٸ� ��⿡ ������� �ʴ� ���� ����� ���ο��� ������ ���Ͽ� ���� ��� ��� ���
        PrivateIncludePaths.AddRange(
			new string[] {
                ModuleDirectory,
				// ... add other private include paths required here ...
			}
			);


        // Public ���Ӽ� ��� �̸� ���(��� �ʿ� ����)(Public/Private ������ �ڵ����� �����).
        //	- �̵��� Public �ҽ� ���Ͽ� �ʿ��� ����Դϴ�.
        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"XmlParser",        // support xml
                "Json",             // support json

                // ... add other public dependencies that you statically link with here ...
                // ���Ӽ� 
                "NxCodeBasePrev",			}
			);


        // Private ���Ӽ� ��� �̸� ����Դϴ�.
        //	- Private �ڵ尡 �����ϴ� ��������� Public Include ���Ͽ����� �ƹ��͵� �������� �ʽ��ϴ�.
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


        // ����� Public ����� �׼����ؾ� �ϴ� ��� ������ ���Ե� ��� �̸�(��δ� �ʿ����� ����)�� ��������� �������ų� ������ �ʿ�� �����ϴ�.
        PublicIncludePathModuleNames.AddRange(
            new string[] {

            });

        // ����� Privae ����� �׼����ؾ� �ϴ� ��� ������ ���Ե� ��� �̸�(��δ� �ʿ����� ����)�� ��������� �������ų� ������ �ʿ�� �����ϴ�.
        PrivateIncludePathModuleNames.AddRange(
            new string[] {

            });

        // ��Ÿ�ӿ� �ʿ��� �� �ִ� �߰� ���
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );

        // Editor ���忡�� ���ԵǱ⸦ ���� ���.
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

        // ����Ƽ ���� �ý��� ��� ���� 
        //  - cpp file ���� ���� �ϳ��� ��� ������ �ӵ��� ������ �ϴ� ���� �ɼ� off ��.
        //  - IWYU Ȯ�� �ÿ��� ������
        bUseUnity = false;
    }
}
