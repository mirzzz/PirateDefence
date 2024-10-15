// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Table/Xml/NxXmlParser.h"
#include "NxXmlFile_App.generated.h"

class FXmlData_AuthInfo
{
public:
	FString Key;
};


class FXmlData_App : public FNxXmlData
{
public:
	float Version;
	FXmlData_AuthInfo AuthInfo;
};

/*
 * ConfigApp.xml 파일을 읽어드릴 parser.
 */
UCLASS()
class NXCODEBASE_API UVxXmlFile_App : public UNxXmlParser
{
	GENERATED_BODY()

public:
	UVxXmlFile_App();
	
	// 수집된 정보 접근 지원.
	virtual bool	     Parsing(FString& filename) override;
	virtual FNxXmlData*  GetData() override { return &XmlData; }
	
protected:
	virtual bool	SaveBinary(FString filename);
	virtual bool	LoadBinary(FString filename);

protected:
	FXmlData_App	XmlData;
};
