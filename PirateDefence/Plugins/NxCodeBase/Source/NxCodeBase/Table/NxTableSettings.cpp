// Fill out your copyright notice in the Description page of Project Settings.


#include "Table/NxTableSettings.h"

#define LOCTEXT_NAMESPACE "NxTableSettings"


UNxTableSettings::UNxTableSettings(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	
}

#if WITH_EDITOR

FText UNxTableSettings::GetSectionText() const
{
	return LOCTEXT("NxTableSettingsName", "Plugin NxTable");
}

FText UNxTableSettings::GetSectionDescription() const
{
	return LOCTEXT("NxTableSettingsDescription", "Xml, Json, StringData, TableData 등 테이블 관련 설정");
}
#endif


#undef LOCTEXT_NAMESPACE