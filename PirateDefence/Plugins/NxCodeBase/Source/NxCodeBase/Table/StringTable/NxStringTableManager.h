// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Internationalization/StringTable.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"

#include "NxBaseEnum.h"
#include "NxTableEnum.h"
#include "NxBaseLog.h"
#include "NxStringTableManager.generated.h"

/**
	UStringTable 관리자
 */
UCLASS(Blueprintable, BlueprintType)
class NXCODEBASE_API UNxStringTableManager : public UObject
{
	GENERATED_BODY()
	
public:
	UNxStringTableManager();
		
	void RegistStringTable(E_StringTable tableType, UStringTable* dataTable);

	//// 형변환 지원 접근
	//template <typename T>
	//T* GetTableRecord(E_StringTable tableType, FString rowName)
	//{
	//	T* returnValue = nullptr;

	//	UStringTable* dataBase = GetTable(tableType);
	//	if (dataBase != nullptr)
	//	{
	//		FString contextStr;
	//		returnValue = dataBase->FindRow<T>(*rowName, contextStr, true /*Mission Log*/);
	//		if (returnValue == nullptr)
	//		{
	//			NxPrintWarning(LogBase, TEXT("Symbol[%s] could not be found in the [%s:%s]"), *rowName, *NxEnum_GetString(E_StringTable, tableType), *contextStr);
	//		}
	//		return returnValue;
	//	}

	//	NxPrintWarning(LogBase, TEXT("Table not registered : %s "), *NxEnum_GetString(E_StringTable, tableType));
	//	return nullptr;
	//}

	//// tableType, Index 을 통한 레코드 반환.
	//template <typename T>
	//T* GetTableRecord(E_StringTable tableType, int32 rowIndex)
	//{
	//	return GetTableRecord<T>(tableType, FString::FromInt(rowIndex));
	//}

	UFUNCTION(BlueprintCallable, Category = Table)
	UStringTable* GetTable(E_StringTable tableType);		// 테이블 접근

	UFUNCTION(BlueprintCallable, Category = Table)
	bool		HasTable(const E_StringTable tableType);	// 테이블 확인

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Table, Meta = (AllowPrivateAccess = true))
	TMap<E_StringTable, TObjectPtr<class UStringTable> > StringTables;
};
 