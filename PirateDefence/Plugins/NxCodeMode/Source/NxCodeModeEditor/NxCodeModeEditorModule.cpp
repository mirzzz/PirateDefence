// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxCodeModeEditorModule.h"

// 로그 구현
DEFINE_LOG_CATEGORY(LogCodeModeEditor);

#define LOCTEXT_NAMESPACE "NxCodeModeEditorModule"


void FNxCodeModeEditorModule::StartupModule()
{
	// 모듈 로딩 시작
	UE_LOG(LogCodeModeEditor, Warning, TEXT("FNxCodeModeEditorModule::StartupModule"));

	FPropertyEditorModule& propertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// 커스커마이싱 대상 클래스 DetailCustomization 등록
	//propertyModule.RegisterCustomClassLayout( ANxMyActor::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FNxMyActorDetailCustomization::MakeInstance));
	

	// 변경 알림
	//propertyModule.NotifyCustomizationModuleChanged();	
}

void FNxCodeModeEditorModule::ShutdownModule()
{
	// 모듈 언로딩 시작
	UE_LOG(LogCodeModeEditor, Warning, TEXT("FNxCodeModeEditorModule::ShutdownModule"));

	if (FModuleManager::Get().IsModuleLoaded(FName("NxCodeModeEditorModule")))
	{
		UE_LOG(LogCodeModeEditor, Warning, TEXT("FNxCodeModeEditorModule Unload...."));
		FPropertyEditorModule& propertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		
		// 커스터마이싱 대상 클래스 DetailCustomization 해재
		//propertyModule.UnregisterCustomClassLayout(ANxMyActor::StaticClass()->GetFName()); 
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNxCodeModeEditorModule, NxCodeModeEditor)