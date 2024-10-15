// Fill out your copyright notice in the Description page of Project Settings.

#include "NxStringTableManager.h"
#include "Engine/DataTable.h"		// UStringTable 지원
#include "Internationalization/StringTable.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"
#include "NxBaseLog.h"

// DataTable Include.

UNxStringTableManager::UNxStringTableManager()
{
}

void UNxStringTableManager::RegistStringTable(E_StringTable tableType, UStringTable* dataTable)
{
	StringTables.Add(tableType, dataTable);

	//FStringTableRegistry::Get().RegisterStringTable(dataTable->GetStringTableId(), dataTable->GetMutableStringTable());
}

// DataTable 접근
UStringTable* UNxStringTableManager::GetTable(E_StringTable tableType)
{
	TObjectPtr<UStringTable> findedElem = StringTables.FindRef(tableType);
	if (findedElem != nullptr)
	{
		return findedElem.Get();
	}

	return nullptr;
}

// DataTable 등록되어 있는지 판단.
bool UNxStringTableManager::HasTable(const E_StringTable tableType)
{
	return StringTables.Contains(tableType);
}
