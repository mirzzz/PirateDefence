// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxLoadingUserWidgetModule..h"
#include "NxLoadingSplashSettings.h"
#include "NxLoadingUserWidgetSettings.h"
#include "NxBaseLog.h"

#define LOCTEXT_NAMESPACE "NxLoadingUserWidgetModule"

void FNxLoadingUserWidgetModule::StartupModule()
{
	// 이 코드는 모듈이 메모리에 로드된 후에 실행됩니다. 정확한 타이밍은 각모듈 LoadingPhase/Type에 의해 .uplugin 파일에 지정됩니다.
	NxPrintTxt(LogLoading, TEXT("[---------------------------------------------------- NxLoadingUserWidgetModule Module Startup]"));

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
	// 이 기능은 모듈을 정리하기 위해 종료 중에 호출될 수 있습니다. 동적 재로딩을 지원하는 모듈의 경우	모듈을 언로딩하기 전에 이 기능을 호출합니다.
	NxPrintTxt(LogLoading, TEXT("[---------------------------------------------------- NxLoadingUserWidgetModule Module Shutdown]"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNxLoadingUserWidgetModule, NxLoadingUserWidget)
