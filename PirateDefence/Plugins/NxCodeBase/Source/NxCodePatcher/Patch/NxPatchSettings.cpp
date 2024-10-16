// Fill out your copyright notice in the Description page of Project Settings.


#include "NxPatchSettings.h"
#include "NxCodePatcherModule.h"
#include "NxBaseLog.h"

#define LOCTEXT_NAMESPACE "NxPatchSettings"

UNxPatchSettings::UNxPatchSettings(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	
}

FString UNxPatchSettings::GetPatchVersionURL(E_DevelopmentType deployType)
{
	FString url;

	// Step1. Settings 값에서 가져오기
	FNxPatchingPath* deployPath = PatchPath.Find(deployType);
	if (deployPath)
	{
		url = deployPath->CdnVersionUrls / FILE_SERVER_VERSION;
	}

	// Step2. xml 파일에서 버전 정보 가져오기 (패키지 된 후, 변경이 필요한 경우 추가 구현)
	NxPrintTodo(LogPatcher, TEXT("Step2. xml 파일에서 Verision 정보 가져오기"));

	return url;
}

int32 UNxPatchSettings::GetPatchDataURL(E_DevelopmentType deployType, TArray<FString>& getArray)
{
	FNxPatchingPath* deployPath = PatchPath.Find(deployType);
	if (deployPath)
	{
		getArray = deployPath->CdnDataUrls;
	}

	// Step3. xml 파일에서 패치 정보 가져오기 (패키지 된 후, 변경이 필요한 경우 추가 구현)
	NxPrintTodo(LogPatcher, TEXT("Step3. xml 파일에서 Patch 정보 가져오기"));

	return getArray.Num();
}

#if WITH_EDITOR

FText UNxPatchSettings::GetSectionText() const
{
	return LOCTEXT("VxPatchSettingsName", "Plugin NxCodePatcher");
}

FText UNxPatchSettings::GetSectionDescription() const
{
	return LOCTEXT("VxPatchSettingsDescription", "Kakao-PlugIn NxCodePatcher 설정");
}
#endif


#undef LOCTEXT_NAMESPACE