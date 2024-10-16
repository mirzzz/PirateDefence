// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxCodeBaseModule.h"
#include "Interfaces/IPluginManager.h"
#include "NxBaseLog.h"

#define LOCTEXT_NAMESPACE "FNxCodeBaseModule"

void FNxCodeBaseModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	NxPrintTxt(LogBase, TEXT("[-------------------------------------------------------- NxCodeBase Module Startup]"));
	FString launchDir = FPaths::LaunchDir();
	FString engineDir = FPaths::EngineDir();
	NxPrintLog(LogBase, TEXT("[ Path::LaunchDir--------------------Absolute> %s"), *launchDir);
	NxPrintLog(LogBase, TEXT("[ Path::EngineDir----------------------------> %s"), *engineDir);

	TSharedPtr<IPlugin> plugin = IPluginManager::Get().FindPlugin(TEXT("NxCodeBase"));
	if (plugin.IsValid())
	{
		FString pluginBaseDir = plugin->GetBaseDir();
		FString pluginContentDir = plugin->GetContentDir();
		NxPrintLog(LogBase, TEXT("[ Path::PluginBaseDir------------------------> %s"), *pluginBaseDir);
		NxPrintLog(LogBase, TEXT("[ Path::PluginContentDir---------------------> %s"), *pluginContentDir);
	}
	NxPrintTxt(LogBase, TEXT("[----------------------------------------------------------------------------------]"));
}

void FNxCodeBaseModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	NxPrintTxt(LogBase, TEXT("[------------------------------------------------------- NxCodeBase Module Shutdown]"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNxCodeBaseModule, NxCodeBase)