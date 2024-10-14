// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxCodeBaseEditorModule.h"

#include "DetailCustomization/NxDetailCustomizationActor.h"
#include "DetailCustomization/NxDetailCustomizationWidget.h"


// 로그 구현
DEFINE_LOG_CATEGORY(LogCodeBaseEditor);


#define LOCTEXT_NAMESPACE "NxCodeBaseEditorModule"


void FNxCodeBaseEditorModule::StartupModule()
{
	// 모듈 로딩 시작
	UE_LOG(LogCodeBaseEditor, Warning, TEXT("FNxCodeBaseEditorModule::StartupModule"));

	FPropertyEditorModule& propertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// DetailCustomization 등록 - 커스커마이싱 대상 클래스 이름 ("NxDetailCustomizationActor")
	propertyModule.RegisterCustomClassLayout( 
		ANxDetailCustomizationActor::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FNxDetailCustomizationWidget::MakeInstance));
	propertyModule.NotifyCustomizationModuleChanged();	// 변경 알림
}

void FNxCodeBaseEditorModule::ShutdownModule()
{
	// 모듈 언로딩 시작
	UE_LOG(LogCodeBaseEditor, Warning, TEXT("FNxCodeBaseEditorModule::ShutdownModule"));

	if (FModuleManager::Get().IsModuleLoaded(FName("NxCodeBaseEditorModule")))
	{
		UE_LOG(LogCodeBaseEditor, Warning, TEXT("FNxCodeBaseEditorModule Unload...."));
		FPropertyEditorModule& propertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		
		// DetailCustomization 해제 - 커스커마이싱 대상 클래스 이름 ("VxDetailCustomizationActor")
		propertyModule.UnregisterCustomClassLayout(ANxDetailCustomizationActor::StaticClass()->GetFName()); 
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNxCodeBaseEditorModule, NxCodeBaseEditor)