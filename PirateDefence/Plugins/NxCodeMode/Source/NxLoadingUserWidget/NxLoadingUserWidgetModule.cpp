// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxLoadingUserWidgetModule..h"
#include "NxLoadingSplashSettings.h"
#include "NxLoadingUserWidgetSettings.h"
#include "NxBaseLog.h"

#define LOCTEXT_NAMESPACE "NxLoadingUserWidgetModule."

void FNxLoadingUserWidgetModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	NxPrintTxt(LogLoading, TEXT("[---------------------------------------------------- WidgetLoading Module Startup]"));

	UNxLoadingSplashSettings*  asyncLoading = GetMutableDefault<UNxLoadingSplashSettings>();
	const UNxLoadingUserWidgetSettings* widgetLoading = GetDefault<UNxLoadingUserWidgetSettings>();
	if (asyncLoading && widgetLoading)
	{
		if (asyncLoading->bUseAsyncLoading && widgetLoading->bUseWidgetLoading)
		{
			asyncLoading->bUseAsyncLoading = false;
		}
	}
}

void FNxLoadingUserWidgetModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	NxPrintTxt(LogLoading, TEXT("[---------------------------------------------------- WidgetLoading Module Shutdown]"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNxLoadingUserWidgetModule, NxLoadingUserWidget)
