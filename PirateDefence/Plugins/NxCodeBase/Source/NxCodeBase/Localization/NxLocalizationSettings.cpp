// Fill out your copyright notice in the Description page of Project Settings.


#include "Localization/NxLocalizationSettings.h"

#define LOCTEXT_NAMESPACE "NxLocalizationSettings"


UNxLocalizationSettings::UNxLocalizationSettings(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	
}

#if WITH_EDITOR

FText UNxLocalizationSettings::GetSectionText() const
{
	return LOCTEXT("NxLocalizationSettingsName", "Plugin NxLocalization");
}

FText UNxLocalizationSettings::GetSectionDescription() const
{
	return LOCTEXT("NxLocalizationSettingsDescription", "NxLocalization 설정");
}
#endif


#undef LOCTEXT_NAMESPACE