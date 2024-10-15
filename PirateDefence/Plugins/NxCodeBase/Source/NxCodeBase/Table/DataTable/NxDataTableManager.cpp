// Fill out your copyright notice in the Description page of Project Settings.

#include "Table/DataTable/NxDataTableManager.h"
#include "Engine/DataTable.h"		// UDataTable 지원
#include "NxBaseLog.h"

// DataTable Include.

UNxDataTableManager::UNxDataTableManager()
{
	
}

void UNxDataTableManager::RegistDataTable(E_DataTable tableType, UDataTable* dataTable)
{
	DataTables.Add(tableType, dataTable);
}

//bool UVxDataTableManager::LoadTablePath(E_DataTable tableType, FString assetName)
//{
//	// DataTable Asset 읽기.
//	UDataTable* dataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *assetName));
//
//	if (dataTable != nullptr)
//	{
//		if (DataTables.Contains(tableType) == false)
//		{
//			// 테이블 타입이 없을 경우 추가.
//			DataTables.Add(tableType, dataTable);
//			return true;
//		}
//		else
//		{
//			// 테이블 타입이 있을 경우. 병합.
//			NxPrintWarning(LogBase, TEXT("UVxDataTableManager:LoadTablePath() Table is Not Merge! <Todo> %d %s"), tableType, *assetName)
//			return true;
//		}
//	}
//	return false;
//}

// DataTable 접근
UDataTable* UNxDataTableManager::GetTable(E_DataTable tableType)
{
	TObjectPtr<UDataTable> findedElem = DataTables.FindRef(tableType);
	if (findedElem != nullptr)
	{
		return findedElem.Get();
	}

	return nullptr;
}

// DataTable 등록되어 있는지 판단.
bool UNxDataTableManager::HasTable(const E_DataTable tableType)
{
	return DataTables.Contains(tableType);
}
