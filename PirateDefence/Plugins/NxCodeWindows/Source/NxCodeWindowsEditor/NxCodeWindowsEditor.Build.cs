
using UnrealBuildTool;
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class NxCodeWindowsEditor : ModuleRules
	{
		public NxCodeWindowsEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

			var EngineDir = Path.GetFullPath(Target.RelativeEnginePath);

            // These nodes are not public so are hard to subclass
            PublicIncludePaths.AddRange(
                new string[] {
				    // ... add public include paths required here ...
				    ModuleDirectory,
                    Path.Combine(ModuleDirectory, "Public"),
                }
                );
            
            PrivateIncludePaths.AddRange(
				new string[] {
                    ModuleDirectory,
					Path.Combine(ModuleDirectory, "Private"),
					// Path.Combine(EngineDir, @"Source/Editor/GraphEditor/Private"),
				}
                );

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "NxCodeBase",
                    "NxCodeWindows"
                    // ... add other public dependencies that you statically link with here ...
                }
                );

            PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add private dependencies that you statically link with here ...
					"CoreUObject",
					"Engine",
					"UnrealEd",
                    "Slate"
                }
                );

            // 유니티 빌드 시스템 기능 끄기 
            //  - cpp file 여러 개를 하나로 묶어서 컴파일 속도를 빠르게 하는 빌드 옵션 off 함.
            //  - IWYU 확인 시에도 유용함
            bUseUnity = false; 
        }
	}
}
