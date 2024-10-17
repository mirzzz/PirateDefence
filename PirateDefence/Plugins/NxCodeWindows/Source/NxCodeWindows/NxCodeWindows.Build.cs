
using System.IO;
using UnrealBuildTool;

public class NxCodeWindows : ModuleRules
{
	public NxCodeWindows(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				ModuleDirectory,
                Path.Combine(ModuleDirectory, "Public"),
            }
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				ModuleDirectory,
                Path.Combine(ModuleDirectory, "Private"),
            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CommonUI",				
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
                "RHI",
                "RenderCore",
				"UMG",
				"HeadMountedDisplay",
				"InputCore",				
				// ... add private dependencies that you statically link with here ...	
								
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
