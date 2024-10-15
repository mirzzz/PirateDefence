// Fill out your copyright notice in the Description page of Project Settings.

#include "Table/Xml/NxXmlManager.h"
#include "Engine/DataTable.h"		// UDataTable 지원
#include "NxBaseLog.h"

// DataTable Include.
#include "Table/Xml/XmlItem/NxXmlFile_Net.h"
#include "Table/Xml/XmlItem/NxXmlFile_App.h"

UNxXmlManager::UNxXmlManager()
{
	
}

bool UNxXmlManager::RegistDataXml(E_XmlData xmlType, FString filename)
{
	UNxXmlParser* newParser = nullptr;

	// 필요한 xml 파서 등록
	switch (xmlType)
	{
	case E_XmlData::ConfigNet:	newParser = NewObject<UNxXmlFile_Net>();	break;
	case E_XmlData::ConfigApp:	newParser = NewObject<UVxXmlFile_App>();	break;
	case E_XmlData::ConfigGame:	newParser = nullptr;						break;

	default:
		return false;
	}

	if (newParser != nullptr && newParser->IsValidLowLevel() )
	{
		if (newParser->Parsing(filename) == true)
		{
			XmlTables.Add(xmlType, newParser);
		}
		else
		{
			NxPrintWarning(LogBase,  TEXT("Parsing Error : %s"), *filename );
			return false;
		}
	}

	return true;
}

// DataXml 접근
UNxXmlParser* UNxXmlManager::GetXml(E_XmlData xmlType)
{
	TObjectPtr<UNxXmlParser> findedElem = XmlTables.FindRef(xmlType);
	if (findedElem != nullptr)
	{
		return findedElem.Get();
	}

	return nullptr;
}

// DataXml 등록되어 있는지 판단.
bool UNxXmlManager::HasXml(const E_XmlData xmlType)
{
	return XmlTables.Contains(xmlType);
}
