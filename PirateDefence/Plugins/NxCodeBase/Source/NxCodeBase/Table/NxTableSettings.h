// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NxTableEnum.h"
#include "NxTableSettings.generated.h"

/**
 * NxTableSubsystem Plugin Setting
 *	- Blueprint 에서 바로 접근을 지원하지 않으므로, UVxTableSettginsHelper를 통해 접근해야 한다.
 */
UCLASS(config = Plugin, DefaultConfig, meta = (DisplayName = "TableSettings"))
class NXCODEBASE_API UNxTableSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// ~UDeveloperSettings
	UNxTableSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FName GetContainerName() const override { return FName("Project"); }	// 프로젝트 설정
	virtual FName GetCategoryName() const override	{ /*return FApp::GetProjectName();*/ return FName("Nx"); }
	virtual FName GetSectionName() const override	{ return FName("NxTable"); }

	 
#if WITH_EDITOR
	// ~UDeveloperSettings
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif

public:

	// DataTable 등록
	UPROPERTY(config, EditAnywhere, Category = Table, meta = ())
	TMap<E_DataTable, TSoftObjectPtr<UDataTable> >  DefaultTables;

	// DataXml 등록 (경로를 Contents 위치에서 상대위치로 설정)
	// https://benui.ca/unreal/uproperty/
	UPROPERTY(config, EditAnywhere, Category = Table, meta = (RelativeToGameDir))
	TMap<E_XmlData, FFilePath>			DefaultXmlFiles;

	// DataJson 등록 (경로를 Contents 위치에서 상대위치로 설정)
	UPROPERTY(config, EditAnywhere, Category = Table, meta = (RelativeToGameDir))
	TMap<E_JsonData, FFilePath>			DefaultJsonFiles;

	// String 테이블 등록
	UPROPERTY(config, EditAnywhere, Category = Table, meta = ())
	TMap<E_StringTable, TSoftObjectPtr<UStringTable> >  DefaultStringTables;
};
