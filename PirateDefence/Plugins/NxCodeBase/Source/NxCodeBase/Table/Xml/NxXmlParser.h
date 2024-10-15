// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Templates/UnrealTypeTraits.h"
#include "XmlNode.h"
#include "NxBaseLog.h"
#include "NxXmlParser.generated.h"


// XML 데이터 기반 클래스
class FNxXmlData {};
 
/*
 * XML 파일 Parsing 기반 클래스
 */
UCLASS(Blueprintable, BlueprintType)
class NXCODEBASE_API UNxXmlParser : public UObject
{
	GENERATED_BODY()

public:
	UNxXmlParser();
	
	// 루트 노드
	const FXmlNode*		GetRootNode();	
	
	// 노드 검색 (처음 검색된 노드 반환)
	FXmlNode*			GetFirstNode(FString nodeTag, bool recursive = true);			

	// 노드 검색 (처음 검색된 노드 반환)
	TArray<FXmlNode*>	GetNodes(FString nodeTag, bool recursive = true);				

	// Access Information

	// 속성값 얻기
	template<typename T>
	T GetAttribute(const FXmlNode* currentNode, FString attributeTag)					
	{
		if (currentNode == nullptr)
			return T();

		FString attributeText = currentNode->GetAttribute(attributeTag);

		// Template Type 확인.
		if (std::is_same<T, int32>::value)			return (T)FCString::Atoi(*attributeText);		// (int32)
		else if (std::is_same<T, int64>::value)		return (T)FCString::Atoi64(*attributeText);		// (int64)
		else if (std::is_same<T, float>::value)		return (T)FCString::Atof(*attributeText);		// (float)
		else if (std::is_same<T, double>::value)	return (T)FCString::Atod(*attributeText);		// (double)
		else if (std::is_same<T, bool>::value)		return (T)FCString::ToBool(*attributeText);		// (bool)
		
#if PLATFORM_WINDOWS
		T result;
		NxPrintWarning(LogBase, TEXT("Does not Support Conversion Type [%s]"), typeid(result).name());
#endif
		return T(0);		
	}

	FString GetAttributeToString(const FXmlNode* currentNode, FString attributeTag);
	
	// 자식노드 속성값 얻기
	template<typename T>
	T GetAttribute(const FXmlNode* parentNode, FString nodeTag, FString attributeTag)	
	{
		const FXmlNode* findNode = parentNode->FindChildNode(nodeTag);
		if (findNode == nullptr)
			return T();

		return GetAttribute<T>(findNode, attributeTag);
	}

	FString GetAttributeToString(const FXmlNode* parentNode, FString nodeTag, FString attributeTag);

	// 자식노드 내용 얻기
	template<typename T>
	T GetContent(const FXmlNode* parentNode, FString nodeTag)							
	{
		const FXmlNode* findNode = parentNode->FindChildNode(nodeTag);
		if (findNode == nullptr)
			return T();

		FString contentText = findNode->GetContent();

		// Template Type으로 변환.
		if (std::is_same<T, int32>::value)			return (T)FCString::Atoi(*contentText);
		else if (std::is_same<T, int64>::value)		return (T)FCString::Atoi64(*contentText);
		else if (std::is_same<T, float>::value)		return (T)FCString::Atof(*contentText);
		else if (std::is_same<T, double>::value)	return (T)FCString::Atod(*contentText);
		else if (std::is_same<T, bool>::value)		return (T)FCString::ToBool(*contentText);
		
#if PLATFORM_WINDOWS
		T result;
		NxPrintWarning(LogBase, TEXT("Does not Support Conversion Type [%s]"), typeid(result).name());
#endif 
		return T(0);
	}
		
	FString GetContentToString(const FXmlNode* parentNode, FString nodeTag);

	// 자식노드들의 내용 배열 얻기
	template<typename T>
	TArray<T> GetContents(const FXmlNode* parentNode, FString nodeTag)
	{
		TArray<T> result;

		const FXmlNode* findNode = parentNode->FindChildNode(nodeTag);
		if (findNode == nullptr)
			return result;

		T value;

		while (findNode != nullptr)
		{
			FString contentText = findNode->GetContent();

			// Template Type 으로 변환.
			if (std::is_same<T, int32>::value)			value = FCString::Atoi(*contentText);
			else if (std::is_same<T, int64>::value)		value = FCString::Atoi64(*contentText);
			else if (std::is_same<T, float>::value)		value = FCString::Atof(*contentText);
			else if (std::is_same<T, double>::value)	value = FCString::Atod(*contentText);
			else if (std::is_same<T, bool>::value)		value = FCString::ToBool(*contentText);
			else
			{
#if PLATFORM_WINDOWS
				NxPrintWarning(LogBase, TEXT("Does not Support Conversion Type [%s]"), typeid(value).name());
#endif
				continue;
			}

			result.Add(value);

			// 다음 줄 검색
			findNode = findNode->GetNextNode();
		}

		return result;
	}

	TArray<FString> GetContentsToString(const FXmlNode* parentNode, FString nodeTag);

	// 수집된 정보 접근 지원.
	virtual FNxXmlData* GetData()					{ return nullptr; }
	virtual bool	    Parsing(FString& filename)	{ return false;   }
	
protected:

	// Xml 파일 읽기
	bool OpenXmlFile(FString filename);
	void Close();

	// Xml > Binary 변환 및 로딩.
	virtual bool SaveBinary(FString filename) { return false; }
	virtual bool LoadBinary(FString filename) { return false; }

	FXmlNode*	GetRecursiveFirstNode(FXmlNode* currentNode, FString nodeTag);							// 재귀 노드 검색
	void		GetRecursiveNodes(FXmlNode* currentNode, FString nodeTag, TArray<FXmlNode*>& nodeSet);	// 재귀 노드 검색

protected:
	FString				 XmlFileName;
	TSharedPtr<FXmlFile> XmlFile;
};
