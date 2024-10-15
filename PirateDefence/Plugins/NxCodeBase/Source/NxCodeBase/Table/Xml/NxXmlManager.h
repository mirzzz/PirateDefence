// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "Engine.h"
//#include "EngineMinimal.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NxTableEnum.h"
#include "NxXmlParser.h"
#include "NxXmlManager.generated.h"

/**
	Xml 파일 관리자
		> Build.cs 에  "XmlParser" 추가 필요
 */
UCLASS(Blueprintable, BlueprintType)
class NXCODEBASE_API UNxXmlManager : public UObject
{
	GENERATED_BODY()
	
public:
	UNxXmlManager();
		
	// 데이터 xml 파일 등록
	bool RegistDataXml(E_XmlData xmlType, FString filename);

	template <typename T>
	T* GetXmlData(E_XmlData xmlType)
	{
		UNxXmlParser* xmlParser = GetXml(xmlType);
		if (nullptr == xmlParser)
			return nullptr;

		return static_cast<T*>(xmlParser->GetData());
	}

	// 데이터 접근
	UFUNCTION(BlueprintCallable, Category = Xml)
	UNxXmlParser*	GetXml(E_XmlData xmlType);			

	UFUNCTION(BlueprintCallable, Category = Xml)
	bool HasXml(const E_XmlData xmlType);

protected:

	// DataXml Registry
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Xml, Meta = (AllowPrivateAccess = true))
	TMap<E_XmlData, TObjectPtr<UNxXmlParser>> XmlTables;
	
};
 