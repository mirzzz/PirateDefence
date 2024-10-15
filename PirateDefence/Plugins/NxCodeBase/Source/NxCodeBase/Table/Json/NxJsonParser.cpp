// Fill out your copyright notice in the Description page of Project Settings.

#include "Table/Json/NxJsonParser.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "NxBaseLog.h"


UNxJsonParser::UNxJsonParser()
{	
}

bool UNxJsonParser::OpenJsonFile(FString filename)
{
	JsonFileName = filename;

	// 파일이 있는지 판단.
	if (IFileManager::Get().FileExists(*JsonFileName) == false)
	{
		//contentPath = FPaths::ProjectContentDir();
		//contentPath.Append(filename);

		JsonFileName = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + filename;

		if( IFileManager::Get().FileExists(*JsonFileName) == false )
		{
			JsonFileName.Empty();
			return false;
		}
	}
		
	// 문자열 읽어들임
	FString	content;
	if( FFileHelper::LoadFileToString(content, *JsonFileName) == false)
		return false;

	// 문자열을 JsonReader에 연결
	TSharedRef<TJsonReader<TCHAR>>	Reader = TJsonReaderFactory<TCHAR>::Create(*content);

	JsonObject = MakeShareable(new FJsonObject());

	// 데이터를 Json 으로 직렬화.
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		return true;
		
		// Object 읽기
		const TSharedPtr<FJsonObject>* TargetObject;
		if (JsonObject->TryGetObjectField(TEXT("FieldName"), TargetObject))
		{ /* When Parsing Success. */ }
		else
		{ /* When Parsing Failed. */ }

		// Number 읽기
		double dValue;
		if (JsonObject->TryGetNumberField(TEXT("FieldName"), dValue))
		{ /* When Parsing Success. */ }
		else
		{ /* When Parsing Failed. */ }
		
		// String 읽기
		FString sValue;
		if (JsonObject->TryGetStringField(TEXT("FieldName"), sValue)) 
		{ /* When Parsing Success. */ }
		else
		{ /* When Parsing Failed. */ }

		// Array 읽기
		const TArray<TSharedPtr<FJsonValue>>* aValue;
		if (JsonObject->TryGetArrayField(TEXT("FieldName"), aValue)) 
		{ /* When Parsing Success. */ }
		else 
		{ /* When Parsing Failed. */ }
	}

	Close();
	return false;
}

void UNxJsonParser::Close()
{
	if (JsonObject.IsValid())
	{
		JsonObject.Reset();
	}
}

const FJsonObject* UNxJsonParser::GetJsonObject()
{
	if (JsonObject != nullptr)
		return JsonObject.Get();

	return nullptr;
}

