// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Table/Xml/NxXmlParser.h"
#include "NxXmlFile_Net.generated.h"


// Connect Info 정보
class FXmlData_ConnectInfo
{
public:
	FString Kind;
	FString Name;
	FString IP;
	int32   Port;
	FString AuthURL;
	
	FString ErrorMsg;

	FXmlData_ConnectInfo() { Clear(); }

	void Clear();
	void PrintLog();	
};

// Account Info 데이터
class FXmlData_AccountInfo
{
public:
	int64		AccountID;			// 계정 ID 
	FString		NickName;
	
	FXmlData_AccountInfo()
	{
		AccountID		= 0;
		NickName		= FString("UnknownUser");
	}
};

class FXmlData_Net : public FNxXmlData
{
public:
	float								Version;
	TArray<FXmlData_ConnectInfo>		Connections;		// Agent 서버
	TMap<int64, FXmlData_AccountInfo>	Accounts;			// Agent 계정	

	FXmlData_ConnectInfo* FindConnect(const FString& evn)
	{
		FXmlData_ConnectInfo* findConnect = nullptr;

		for (FXmlData_ConnectInfo& connect : Connections)
		{
			if (connect.Name.Compare(evn, ESearchCase::IgnoreCase) == 0)
			{
				findConnect = &connect;
				break;
			}
		}
		return findConnect;
	}
};

/*
 * ConfigNet.xml 파일을 읽어드릴 parser.
 */
UCLASS()
class NXCODEBASE_API UNxXmlFile_Net : public UNxXmlParser
{
	GENERATED_BODY()

public:
	UNxXmlFile_Net();
	
	// 수집된 정보 접근 지원.
	virtual bool	     Parsing(FString& filename) override;
	virtual FNxXmlData*  GetData() override { return &XmlData; }
	
protected:
	virtual bool	SaveBinary(FString filename);
	virtual bool	LoadBinary(FString filename);

protected:
	FXmlData_Net	XmlData;
};
