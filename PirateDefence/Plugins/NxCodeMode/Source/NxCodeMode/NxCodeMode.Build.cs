// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class NxCodeMode : ModuleRules
{
	public NxCodeMode(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				ModuleDirectory,
            }
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				ModuleDirectory,
            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			
				// ... add other public dependencies that you statically link with here ...

				// ���Ӽ� ����.				
                "NxCodeBase",
            });
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...					

				"Projects",		// Plugin �ڵ� ����.
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
