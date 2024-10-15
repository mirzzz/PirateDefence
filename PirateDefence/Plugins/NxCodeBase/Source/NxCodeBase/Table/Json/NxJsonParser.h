// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Templates/UnrealTypeTraits.h"
#include "XmlNode.h"
#include "NxJsonParser.generated.h"


// Json 데이터 기반 클래스
class FNxJsonData {};
 
/*
 * Json 파일 Parsing 기반 클래스
 */
UCLASS(Blueprintable, BlueprintType)
class NXCODEBASE_API UNxJsonParser : public UObject
{
	GENERATED_BODY()

public:
	UNxJsonParser();

	const class FJsonObject* GetJsonObject();
		
	// 수집된 정보 접근 지원.
	virtual bool	     Parsing(FString& filename) { return false; }
	virtual FNxJsonData* GetData()					{ return nullptr; }
	virtual void		 GenerateData()				{}

protected:

	// Xml 파일 읽기
	bool OpenJsonFile(FString filename);
	void Close();

	// Xml > Binary 변환 및 로딩.
	virtual bool SaveBinary(FString filename) { return false; }
	virtual bool LoadBinary(FString filename) { return false; }

protected:
	FString							JsonFileName;
	TSharedPtr<class FJsonObject>	JsonObject;
	
};
