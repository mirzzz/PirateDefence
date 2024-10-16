// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxCodeModeModule.h"
#include "Interfaces/IPluginManager.h"
#include "NxBaseLog.h"

#define LOCTEXT_NAMESPACE "FNxCodeModeModule"

void FNxCodeModeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	NxPrintTxt(LogMode, TEXT("[-------------------------------------------------------- NxCodeMode Module Startup]"));
	FString launchDir = FPaths::LaunchDir();
	FString engineDir = FPaths::EngineDir();
	NxPrintLog(LogMode, TEXT("[ Path::LaunchDir--------------------Absolute> %s"), *launchDir);
	NxPrintLog(LogMode, TEXT("[ Path::EngineDir----------------------------> %s"), *engineDir);

	TSharedPtr<IPlugin> plugin = IPluginManager::Get().FindPlugin(TEXT("NxCodeMode"));
	if (plugin.IsValid())
	{
		FString pluginBaseDir = plugin->GetBaseDir();
		FString pluginContentDir = plugin->GetContentDir();
		NxPrintLog(LogMode, TEXT("[ Path::PluginBaseDir------------------------> %s"), *pluginBaseDir);
		NxPrintLog(LogMode, TEXT("[ Path::PluginContentDir---------------------> %s"), *pluginContentDir);
	}
	NxPrintTxt(LogMode, TEXT("[----------------------------------------------------------------------------------]"));
}

void FNxCodeModeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	NxPrintTxt(LogBase, TEXT("[------------------------------------------------------- NxCodeMode Module Shutdown]"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNxCodeModeModule, NxCodeMode)