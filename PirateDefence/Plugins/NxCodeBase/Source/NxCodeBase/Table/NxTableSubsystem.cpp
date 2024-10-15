// Copyright Epic Games, Inc. All Rights Reserved.

#include "Table/NxTableSubsystem.h"

#include "GameFramework/HUD.h"
#include "Misc/AssertionMacros.h"
#include "Engine/DataTable.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/Paths.h"

#include "Internationalization/StringTable.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"
#include "Engine/AssetManager.h"

#include "NxGameInstanceBase.h"
#include "NxTableEnum.h"

#include "Table/NxTableSettings.h"
#include "Table/DataTable/NxDataTableManager.h"
#include "Table/Xml/NxXmlManager.h"
#include "Table/Xml/NxXmlParser.h"
#include "Table/Xml/XmlItem/NxXmlFile_Net.h"
#include "Table/Json/NxJsonManager.h"
#include "Table/Json/NxJsonParser.h"
#include "Table/Json/JsonItem/NxJsonFile_Spline.h"
#include "NxBaseLog.h"



/*static*/ UNxTableSubsystem* UNxTableSubsystem::Get()
{
	return UNxGameInstanceBase::GetGameInstanceSubsystem<UNxTableSubsystem>();
}

bool UNxTableSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> ChildClasses;

	GetDerivedClasses(GetClass(), ChildClasses, false);

	// 다른 곳에 정의된 재정의 구현이 없는 경우에만 인스턴스를 만듭니다. (외부에서 생성시 기본 시스템은 생성하지 않음)
	return ChildClasses.Num() == 0;
}

void UNxTableSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{							 
	NxPrintTxt(LogBase, TEXT("[-------------------------------------------------------- TableSubsystem Initialize]"));

	Super::Initialize(Collection);
		
	// 1초에 한번 호출.
	Handle_DelayTick = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UNxTableSubsystem::DelayTick), 1.0f);

	// 데이터 테이블 관리자 등록
	TablesRegistry.Add(E_TableCategory::DataTable, NewObject<UNxDataTableManager>());
	TablesRegistry.Add(E_TableCategory::DataXml, NewObject<UNxXmlManager>());
	TablesRegistry.Add(E_TableCategory::DataJson, NewObject<UNxJsonManager>());
	TablesRegistry.Add(E_TableCategory::StringTable, NewObject<UNxStringTableManager>());

	// 데이터 테이블 구성	
	if (const UNxTableSettings* tableSettings = GetDefault<UNxTableSettings>())
	{
		if (tableSettings->DefaultTables.Num() > 0)
		{
			// DataTable
			for (auto& tableItem : tableSettings->DefaultTables)
			{
				//if (UObject* tableObj = tableItem.Value.TryLoad())		// FSoftObjectPath 사용시
				if (UObject* tableObj = tableItem.Value.LoadSynchronous())	// TSoftObjectPtr 사용시
				{
					if (UDataTable* dataTable = Cast<UDataTable>(tableObj))
					{
						// 설정한 테이블 등록
						GetManager<UNxDataTableManager>(E_TableCategory::DataTable)->RegistDataTable(tableItem.Key, dataTable);
					}
					else
					{
						ensureMsgf(dataTable, TEXT("TabelSettings Missing!"));
					}
				}
			}
		}

		// xml
		if (tableSettings->DefaultXmlFiles.Num() > 0)
		{
			// Xml
			for (auto& xmlItem : tableSettings->DefaultXmlFiles)
			{
				FString xmlabsolutPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + xmlItem.Value.FilePath;
				
				GetManager<UNxXmlManager>(E_TableCategory::DataXml)->RegistDataXml(xmlItem.Key, xmlabsolutPath);
			}

			// Test Output Print
			if (FXmlData_Net* netData = GetXmlData<FXmlData_Net>(E_XmlData::ConfigNet))
			{
				for (int32 i = 0; i < netData->Connections.Num(); ++i)
				{
					netData->Connections[i].PrintLog();
				}
			}
		}

		// Json
		if (tableSettings->DefaultJsonFiles.Num() > 0)
		{
			// Xml
			for (auto& jsonItem : tableSettings->DefaultJsonFiles)
			{
				FString jsonabsolutPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + jsonItem.Value.FilePath;
								
				GetManager<UNxJsonManager>(E_TableCategory::DataJson)->RegistDataJson(jsonItem.Key, jsonItem.Value.FilePath);
			}

			// Test Output Print
			if (FJsonData_Spline* splineData = GetJsonData<FJsonData_Spline>(E_JsonData::CourseSpline))
			{
				for (int32 i = 0; i < splineData->ArrSet.Num(); ++i)
				{
					splineData->ArrSet[i].PrintLog();
				}
			}
		}

		//UStringTable
		if (tableSettings->DefaultStringTables.Num() > 0)
		{
			for (auto& tableItem : tableSettings->DefaultStringTables)
			{
				//if (UObject* tableObj = tableItem.Value.TryLoad())		// FSoftObjectPath 사용시
				if (UObject* tableObj = tableItem.Value.LoadSynchronous())	// TSoftObjectPtr 사용시
				{
					if (UStringTable* dataTable = Cast<UStringTable>(tableObj))
					{
						GetManager<UNxStringTableManager>(E_TableCategory::StringTable)->RegistStringTable(tableItem.Key, dataTable);
					}
					else
					{
						ensureMsgf(dataTable, TEXT("TabelSettings Missing!"));
					}
				}
			}
		}
	}
}

void UNxTableSubsystem::Deinitialize()
{
	TablesRegistry.Empty();

	FTSTicker::GetCoreTicker().RemoveTicker(Handle_DelayTick);

	Super::Deinitialize();
		
	NxPrintTxt(LogBase, TEXT("[------------------------------------------------------------- TableSubsystem Deinitialize]"));
}

UDataTable* UNxTableSubsystem::GetDataTable(E_DataTable tableType)
{
	UNxDataTableManager* dataTables = GetManager<UNxDataTableManager>(E_TableCategory::DataTable);
	if (dataTables != nullptr)
	{
		return dataTables->GetTable(tableType);
	}
	return nullptr;
}

bool UNxTableSubsystem::DelayTick(float DeltaTime)
{
	NxPrintFunc(LogBase);
	
	// Tick을 해제 (1회후 해제)
	FTSTicker::GetCoreTicker().RemoveTicker(Handle_DelayTick);

	return true;
}


#if WITH_EDITOR

// 디렉토리 열기
bool UNxTableSubsystem::GetDirectoryFromDialog(const FString& DialogTitle, const FString& DefaultPath, FString& OutFileFolderName)
{
	bool bOpenDirectory = false;

	//void* ParentWindowHandle = GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		DesktopPlatform->OpenDirectoryDialog(
			ParentWindowHandle,
			DialogTitle,
			DefaultPath,
			OutFileFolderName);

		bOpenDirectory = (OutFileFolderName.Len() > 0);
	}

	return bOpenDirectory;
}


// 파일 열기
bool UNxTableSubsystem::GetFileFromDialog(const FString& DialogTitle, const FString& DefaultPath, const FString& FileTypes, TArray<FString>& OutFileNames)
{
	bool bOpenFile = false;

	//void* ParentWindowHandle = GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		DesktopPlatform->OpenFileDialog(
			ParentWindowHandle,
			DialogTitle,
			DefaultPath,
			FString(""),
			FileTypes,
			EFileDialogFlags::None,
			OutFileNames);

		bOpenFile = (OutFileNames.Num() > 0);
	}

	return bOpenFile;
}

#endif // WITH_EDITOR