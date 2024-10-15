// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "NxBaseEnum.h"
#include "NxTableEnum.h"
#include "NxBaseLog.h"
#include "NxDataTableManager.generated.h"


/**
	UDatatable 관리자
 */
UCLASS(Blueprintable, BlueprintType)
class NXCODEBASE_API UNxDataTableManager : public UObject
{
	GENERATED_BODY()
	
public:
	UNxDataTableManager();
		
	// 데이터 테이블 등록
	void RegistDataTable(E_DataTable tableType, UDataTable* dataTable);

	// 형변환 지원 접근
	template <typename T>
	T* GetTableRecord(E_DataTable tableType, FString rowName)
	{
		T* returnValue = nullptr;

		UDataTable* dataBase = GetTable(tableType);
		if (dataBase != nullptr)
		{
			FString contextStr;
			returnValue = dataBase->FindRow<T>(*rowName, contextStr, true /*Mission Log*/);
			if (returnValue == nullptr)
			{
				NxPrintWarning(LogBase, TEXT("Symbol[%s] could not be found in the [%s:%s]"), *rowName, *NxEnum_GetString(E_DataTable, tableType), *contextStr);
			}
			return returnValue;
		}

		NxPrintWarning(LogBase, TEXT("Table not registered : %s "), *NxEnum_GetString(E_DataTable, tableType) );
		return nullptr;
	}


	// tableType, Index 을 통한 레코드 반환.
	template <typename T>
	T* GetTableRecord(E_DataTable tableType, int32 rowIndex)
	{
		return GetTableRecord<T>(tableType, FString::FromInt(rowIndex));
	}

	UFUNCTION(BlueprintCallable, Category = Table)
	UDataTable* GetTable(E_DataTable tableType);			// 테이블 접근

	UFUNCTION(BlueprintCallable, Category = Table)
	bool		HasTable(const E_DataTable tableType);		// 테이블 확인

protected:

	// DataTable Registry
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Table, Meta = (AllowPrivateAccess = true))
	TMap<E_DataTable, TObjectPtr<class UDataTable> > DataTables;

	
};
 