// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class NxCodeBasePrev : ModuleRules
{
	public NxCodeBasePrev(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] 
			{
				// ... add public include paths required here ...
				ModuleDirectory,
				// �ܺ� ��⿡�� ��θ� ���� �ʰ� ��� ������ �ٷ� Include �Ϸ��� ��� �߰� ����.
                //Path.Combine(ModuleDirectory, "Public"),
            }
		);


        PrivateIncludePaths.AddRange(
            new string[]
			{
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
			}
		);

        PublicIncludePathModuleNames.AddRange(
            new string[]
			{

            }
        );
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{

			}
		);

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
		);

        // ����Ƽ ���� �ý��� ��� ���� 
        //  - cpp file ���� ���� �ϳ��� ��� ������ �ӵ��� ������ �ϴ� ���� �ɼ� off ��.
        //  - IWYU Ȯ�� �ÿ��� ������
        bUseUnity = false;
    }
}
