// Copyright Epic Games, Inc. All Rights Reserved.

#include "NxCodePatcherLibrary.h"
#include "NxPatchSettings.h"

#define LOCTEXT_NAMESPACE "NxCodePatcherLibrary"


UNxCodePatcherLibrary::UNxCodePatcherLibrary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

bool UNxCodePatcherLibrary::GetPatchEnable()
{
	return GetDefault<UNxPatchSettings>()->PatchEnable;
}

E_DevelopmentType UNxCodePatcherLibrary::GetPatchDevelopType()
{
	return GetDefault<UNxPatchSettings>()->DeployType;
}

TSoftClassPtr<UUserWidget> UNxCodePatcherLibrary::GetPatchWidgetClass()
{
	TSoftClassPtr<UUserWidget> SoftClassWidget = GetDefault<UNxPatchSettings>()->WidgetClass;
	return SoftClassWidget;
}

TSoftObjectPtr<UWorld> UNxCodePatcherLibrary::GetNextMapAfterPatching()
{
	// TSoftObjectPtr<> 간접 참조, 레퍼런스가 아닌 UClass를 참조한다. (과거 TAssetSubclassOf 에서 대체됨)
	TSoftObjectPtr<UWorld> SoftObjectWorld = GetDefault<UNxPatchSettings>()->NextMapAfterPatching;
	return SoftObjectWorld;
}

#undef LOCTEXT_NAMESPACE //"NxCodePatcherLibrary"

