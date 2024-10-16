// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxLoadingUserWidgetModule..h"
#include "NxLoadingSplashSettings.h"
#include "NxLoadingUserWidgetSettings.h"
#include "NxBaseLog.h"

#define LOCTEXT_NAMESPACE "NxLoadingUserWidgetModule"

void FNxLoadingUserWidgetModule::StartupModule()
{
	// �� �ڵ�� ����� �޸𸮿� �ε�� �Ŀ� ����˴ϴ�. ��Ȯ�� Ÿ�̹��� ����� LoadingPhase/Type�� ���� .uplugin ���Ͽ� �����˴ϴ�.
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
	// �� ����� ����� �����ϱ� ���� ���� �߿� ȣ��� �� �ֽ��ϴ�. ���� ��ε��� �����ϴ� ����� ���	����� ��ε��ϱ� ���� �� ����� ȣ���մϴ�.
	NxPrintTxt(LogLoading, TEXT("[---------------------------------------------------- NxLoadingUserWidgetModule Module Shutdown]"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNxLoadingUserWidgetModule, NxLoadingUserWidget)
