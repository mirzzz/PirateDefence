// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "NxTableEnum.h"
#include "DataTable/NxDataTableManager.h"
#include "Xml/NxXmlManager.h"
#include "Json/NxJsonManager.h"
#include "StringTable/NxStringTableManager.h"
#include "NxTableSubsystem.generated.h"


/* 
* 테이블 관리자.
*   - DataTable : read (Unreal DataTable Asset)
*	- xml		: read/write	 
*	- json		: read/write
*/
UCLASS()
class NXCODEBASE_API UNxTableSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UNxTableSubsystem* Get();

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 형변환 Manager 접근 지원 (데이터 종류 : DataTable, Xml, Json ... )
	template <typename T>
	T* GetManager(E_TableCategory tableType)
	{
		T* returnValue = nullptr;

		if (TablesRegistry.Contains(tableType) == true)
		{
			returnValue = static_cast<T*>(TablesRegistry[tableType]);
		}

		return returnValue;
	}


	// DataTabel : Fast Access
	//------------------------------------------------------------------------------		
	class UDataTable* GetDataTable(E_DataTable tableType);

	// index 검색
	template <typename T>
	T* GetTableRecord(E_DataTable tableType, int32 rowIndex)
	{
		UNxDataTableManager* dataTables = GetManager<UNxDataTableManager>(E_TableCategory::DataTable);
		if (dataTables != nullptr)
		{
			T* findTable = dataTables->GetTableRecord<T>(tableType, rowIndex);
			if (findTable != nullptr)
			{
				return findTable;
			}
		}
		return nullptr;
	}

	// Name 검색
	template <typename T>
	T* GetTableRecord(E_DataTable tableType, FString rowName)
	{
		if (UNxDataTableManager* tableMgr = GetManager<UNxDataTableManager>(E_TableCategory::DataTable))
		{
			T* findTable = tableMgr->GetTableRecord<T>(tableType, rowName);
			if (findTable != nullptr)
			{
				return findTable;
			}
		}
		return nullptr;
	}


	// Xml Fast Access
	//------------------------------------------------------------------------------
	template <typename T>
	T* GetXmlData(E_XmlData xmlType)
	{
		if (UNxXmlManager* xmlMgr = GetManager<UNxXmlManager>(E_TableCategory::DataXml))
		{
			return xmlMgr->GetXmlData<T>(xmlType);
		}
		return nullptr;
	}

	// Json Fast Access
	//------------------------------------------------------------------------------
	template <typename T>
	T* GetJsonData(E_JsonData jsonType)
	{
		if (UNxJsonManager* jsonMgr = GetManager<UNxJsonManager>(E_TableCategory::DataJson))
		{
			return jsonMgr->GetJsonData<T>(jsonType);
		}
		return nullptr;
	}

	//StringTable
	//------------------------------------------------------------------------------
	FName GetStringTableId(E_StringTable stringTableType)
	{
		if (UNxStringTableManager* manager = GetManager<UNxStringTableManager>(E_TableCategory::StringTable))
		{
			auto stringTable = manager->GetTable(stringTableType);
			if (nullptr != stringTable)
				return stringTable->GetStringTableId();

		}
		return FName();
	}

#if WITH_EDITOR

	// 다이얼로그를 통해 디렉토리/파일 리스트 얻기
	bool GetDirectoryFromDialog(const FString& DialogTitle, const FString& DefaultPath, FString& OutFileFolderName);
	bool GetFileFromDialog(const FString& DialogTitle, const FString& DefaultPath, const FString& FileTypes, TArray<FString>& OutFileNames);

#endif  // WITH_EDITOR

private:

	bool DelayTick(float DeltaTime);
	
	FTSTicker::FDelegateHandle  Handle_DelayTick;

protected:
	
	// Table 정보.
	UPROPERTY()
	TMap<E_TableCategory, TObjectPtr<UObject>> TablesRegistry;
		
};