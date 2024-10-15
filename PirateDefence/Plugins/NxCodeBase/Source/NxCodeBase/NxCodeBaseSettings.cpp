// Fill out your copyright notice in the Description page of Project Settings.


#include "NxCodeBaseSettings.h"
#include "NxBaseEnum.h"

#define LOCTEXT_NAMESPACE "NxCodeBaseSettings"


UNxCodeBaseSettings::UNxCodeBaseSettings(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{

}

#if WITH_EDITOR

FText UNxCodeBaseSettings::GetSectionText() const
{
	return LOCTEXT("NxCodeBaseSettingsName", "Plugin NxCodeBase");
}

FText UNxCodeBaseSettings::GetSectionDescription() const
{
	return LOCTEXT("NxCodeBaseSettingsDescription", "CodeBase에 필요한 설정");
}
#endif



#undef LOCTEXT_NAMESPACE