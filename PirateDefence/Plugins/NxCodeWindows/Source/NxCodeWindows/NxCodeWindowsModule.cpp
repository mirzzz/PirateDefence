
#include "NxCodeWindowsModule.h"
#include "Widgets/SWindow.h"
#include "GenericPlatform/GenericApplication.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "NxCodeWindowsManager.h"
#include "NxBaseLog.h"

DEFINE_LOG_CATEGORY(LogCodeWindows);

#define LOCTEXT_NAMESPACE "NxCodeWindowsModule"

void FNxCodeWindowsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FNxCodeWindowsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FNxCodeWindowsModule::GetMultiWindowsManager(UNxCodeWindowsManager*& multiWindowsManager)
{
	multiWindowsManager = nullptr;
	if (!GEngine)
	{
		return;
	}

	// 오브젝트 Index 로 Manager 검색.
	FUObjectItem* objectItem = GUObjectArray.IndexToObject(MultiWindowsManagerObjectIndex);
	if (objectItem && objectItem->Object)
	{
		UObject* object = static_cast<UObject*>(objectItem->Object);
		multiWindowsManager = Cast<UNxCodeWindowsManager>(object);
	}

	// 아직 생성이 안된 경우, 생성 및 Index 저장.
	if (!multiWindowsManager || UNxCodeWindowsManager::StaticClass() != multiWindowsManager->StaticClass())
	{
		// GC 대상에서 제외
		multiWindowsManager = NewObject<UNxCodeWindowsManager>(UNxCodeWindowsManager::StaticClass());
		multiWindowsManager->AddToRoot();

		MultiWindowsManagerObjectIndex = GUObjectArray.ObjectToIndex(multiWindowsManager);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNxCodeWindowsModule, NxCodeWindows)