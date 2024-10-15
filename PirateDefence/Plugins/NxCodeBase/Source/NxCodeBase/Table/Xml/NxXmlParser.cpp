// Fill out your copyright notice in the Description page of Project Settings.

#include "Table/Xml/NxXmlParser.h"
#include "NxBaseLog.h"
#include "XmlFile.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

UNxXmlParser::UNxXmlParser()
{	
}

bool UNxXmlParser::OpenXmlFile(FString filename)
{
	XmlFileName = filename;

	// 파일이 있는지 판단.
	if (IFileManager::Get().FileExists(*XmlFileName) == false)
	{
		//contentPath = FPaths::ProjectContentDir();
		//contentPath.Append(filename);

		XmlFileName = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + filename;

		if( IFileManager::Get().FileExists(*XmlFileName) == false )
		{
			XmlFileName.Empty();
			return false;
		}
	}

	XmlFile = MakeShareable( new FXmlFile() );
	if (XmlFile.IsValid() == false)
		return false;

	// 파일 로드
	if (XmlFile->LoadFile(XmlFileName) == false)
	{
		NxPrintWarning(LogBase, TEXT("Xml File [%s] is not Load!"), *XmlFileName);
		XmlFile.Reset();
		return false;
	}

	// 파일 유효성 검사.
	if (XmlFile->IsValid() == false)
	{
		NxPrintWarning(LogBase, TEXT("Xml File [%s] is not Valid! errCode[%d]"), *XmlFileName, *XmlFile->GetLastError());
		XmlFile.Reset();
		return false;
	}

	return true;
}

void UNxXmlParser::Close()
{
	if (XmlFile.IsValid())
	{
		XmlFile.Reset();
	}
}

const FXmlNode* UNxXmlParser::GetRootNode()
{
	if(XmlFile != nullptr)
		return XmlFile->GetRootNode();

	return nullptr;
}

// 노드 검색
FXmlNode* UNxXmlParser::GetFirstNode(FString nodeTag, bool recursive /*= true*/)
{
	FXmlNode* pRootNode = XmlFile->GetRootNode();

	if (recursive == false)
	{
		if (pRootNode->GetTag().Equals(nodeTag) == true)
			return pRootNode;

		FXmlNode* childrenNode = pRootNode->FindChildNode(nodeTag);
		return childrenNode;
	}

	return GetRecursiveFirstNode(pRootNode, nodeTag);
}

// 재귀 노드 검색
FXmlNode* UNxXmlParser::GetRecursiveFirstNode(FXmlNode* currentNode, FString nodeTag)
{
	if (currentNode == nullptr)
		return nullptr;

	if( currentNode->GetTag().Equals(nodeTag) == true )
		return currentNode;

	const TArray<FXmlNode*> childrenNodes = currentNode->GetChildrenNodes();
	for (auto& node : childrenNodes)
	{
		if (node != nullptr)
		{
			FXmlNode* findChildNode = GetRecursiveFirstNode(node, nodeTag);
			if (findChildNode != nullptr)
				return findChildNode;
		}
	}
	return nullptr;
}

TArray<FXmlNode*> UNxXmlParser::GetNodes(FString nodeTag, bool recursive /*= true*/)
{
	TArray<FXmlNode*> nodeSet;

	FXmlNode* pRootNode = XmlFile->GetRootNode();

	if (recursive == false)
	{
		if (pRootNode->GetTag().Equals(nodeTag) == true) 
			nodeSet.Add(pRootNode);

		const TArray<FXmlNode*> childrenNodes = pRootNode->GetChildrenNodes();
		for (auto& node : childrenNodes)
		{
			if (node->GetTag().Equals(nodeTag) == true)
				nodeSet.Add(node);
		}
		return nodeSet;
	}

	GetRecursiveNodes(pRootNode, nodeTag, nodeSet);
	return nodeSet;
}

FString UNxXmlParser::GetAttributeToString(const FXmlNode* currentNode, FString attributeTag)
{
	if (currentNode == nullptr)
		return TEXT("");

	FString result = currentNode->GetAttribute(attributeTag);
	return result;

	// int <> FString 으로 변환이 안됨.
	//{
	//  FString attributeText = currentNode->GetAttribute(attributeTag);
	//  T result;
	//	FString resultType = ANSI_TO_TCHAR(typeid(result).name());
	//	if(resultType.Equals(TEXT("FString")) == true)	
	//		result = attributeText;
	//}	
}

FString UNxXmlParser::GetAttributeToString(const FXmlNode* parentNode, FString nodeTag, FString attributeTag)
{
	const FXmlNode* findNode = parentNode->FindChildNode(nodeTag);
	if (findNode == nullptr)
		return TEXT("");

	return GetAttributeToString(findNode, attributeTag);
}


FString UNxXmlParser::GetContentToString(const FXmlNode* parentNode, FString nodeTag)
{
	const FXmlNode* findNode = parentNode->FindChildNode(nodeTag);
	if (findNode == nullptr)
		return TEXT("");

	FString result = findNode->GetContent();
	return result;
}


TArray<FString> UNxXmlParser::GetContentsToString(const FXmlNode* parentNode, FString nodeTag)
{
	TArray<FString> result;

	const FXmlNode* findNode = parentNode->FindChildNode(nodeTag);
	if (findNode == nullptr)
		return result;

	FString  value;

	while (findNode != nullptr)
	{
		value = findNode->GetContent();
		result.Add(value);

		// 다음 줄 검색
		findNode = findNode->GetNextNode();
	}

	return result;
}

void UNxXmlParser::GetRecursiveNodes(FXmlNode* currentNode, FString nodeTag, TArray<FXmlNode*>& nodeSet)
{
	if (currentNode == nullptr)
		return;

	if (currentNode->GetTag().Equals(nodeTag) == true)
		nodeSet.Add(currentNode);

	const TArray<FXmlNode*> childrenNodes = currentNode->GetChildrenNodes();
	for (auto& node : childrenNodes)
	{
		if (node != nullptr)
			GetRecursiveNodes(node, nodeTag, nodeSet);
	}
	return;
}
