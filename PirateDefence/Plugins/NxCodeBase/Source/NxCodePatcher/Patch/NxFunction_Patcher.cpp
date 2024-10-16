// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxFunction_Patcher.h"
#include "NxPatchSettings.h"

#define LOCTEXT_NAMESPACE "NxFunction_Patcher"


UNxFunction_Patcher::UNxFunction_Patcher(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

bool UNxFunction_Patcher::GetPatchEnable()
{
	return GetDefault<UNxPatchSettings>()->PatchEnable;
}

E_DevelopmentType UNxFunction_Patcher::GetPatchDevelopType()
{
	return GetDefault<UNxPatchSettings>()->DeployType;
}

TSoftClassPtr<UUserWidget> UNxFunction_Patcher::GetPatchWidgetClass()
{
	TSoftClassPtr<UUserWidget> SoftClassWidget = GetDefault<UNxPatchSettings>()->WidgetClass;
	return SoftClassWidget;
}

TSoftObjectPtr<UWorld> UNxFunction_Patcher::GetNextMapAfterPatching()
{
	// TSoftObjectPtr<> 간접 참조, 레퍼런스가 아닌 UClass를 참조한다. (과거 TAssetSubclassOf 에서 대체됨)
	TSoftObjectPtr<UWorld> SoftObjectWorld = GetDefault<UNxPatchSettings>()->NextMapAfterPatching;
	return SoftObjectWorld;
}

#undef LOCTEXT_NAMESPACE //"NxFunction_Patcher"

