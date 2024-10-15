// Fill out your copyright notice in the Description page of Project Settings.

#include "Table/Json/NxJsonManager.h"
#include "NxBaseLog.h"

#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Serialization/JsonWriter.h"
#include "HAL/PlatformFileManager.h"

#include "Table/NxTableSubsystem.h"
#include "Table/Json/JsonItem/NxJsonFile_Spline.h"

UNxJsonManager::UNxJsonManager()
{
	
}

#if WITH_EDITOR
void UNxJsonManager::RegistDataJsonFormDialog()
{
	FString dialogTitle = TEXT("Get JsonFiles");
	FString defaultPath = TEXT("Game");
	FString fileTypes	= TEXT("json (*.json)|*.json");

	TArray<FString> SelectedFiles;

	if (UNxTableSubsystem::Get()->GetFileFromDialog(dialogTitle, defaultPath, fileTypes, SelectedFiles) == false)
		return;
	
	if (SelectedFiles.Num() <= 0)
		return;

	for (int32 fileidx = 0; fileidx < SelectedFiles.Num(); ++fileidx)
	{
		FString filename = SelectedFiles[fileidx];

		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*filename) == false)
			continue;

		// E_DataJson 확인 필요.
		// E_DataJson jsonType = E_DataJson::None;
		// RegistDataJson(jsonType, filename);
	}
}
#endif // WITH_EDITOR

bool UNxJsonManager::RegistDataJson(E_JsonData jsonType, FString filename)
{
	UNxJsonParser* newParser = nullptr;

	// 필요한 json 파서 등록
	switch (jsonType)
	{
	case E_JsonData::CourseSpline:	newParser = NewObject<UNxJsonFile_Spline>();	break;

	default:
		return false;
	}

	if (newParser != nullptr && newParser->IsValidLowLevel())
	{
		if (newParser->Parsing(filename) == true)
		{
			JsonTables.Add(jsonType, newParser);

			// 재구성 필요시 호출
			newParser->GenerateData();
		}
		else
		{
			NxPrintWarning(LogBase, TEXT("Parsing Error : %s"), *filename);
			return false;
		}
	}

	return true;
}

class UNxJsonParser* UNxJsonManager::GetJson(E_JsonData jsonType)
{
	TObjectPtr<UNxJsonParser> findedElem = JsonTables.FindRef(jsonType);
	if (findedElem != nullptr)
	{
		return findedElem.Get();
	}

	return nullptr;
}

bool UNxJsonManager::HasJson(const E_JsonData jsonType)
{
	return JsonTables.Contains(jsonType);
}

