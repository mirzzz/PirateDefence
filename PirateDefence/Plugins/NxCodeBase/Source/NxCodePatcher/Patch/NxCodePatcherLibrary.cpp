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
	// TSoftObjectPtr<> ���� ����, ���۷����� �ƴ� UClass�� �����Ѵ�. (���� TAssetSubclassOf ���� ��ü��)
	TSoftObjectPtr<UWorld> SoftObjectWorld = GetDefault<UNxPatchSettings>()->NextMapAfterPatching;
	return SoftObjectWorld;
}

#undef LOCTEXT_NAMESPACE //"NxCodePatcherLibrary"

