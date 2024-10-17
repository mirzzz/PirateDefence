// Fill out your copyright notice in the Description page of Project Settings.


#include "NxCodeUISettings.h"

#include "CommonBorder.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"

#define LOCTEXT_NAMESPACE "NxCodeUISettings"

TSubclassOf<UCommonBorderStyle> FNxStylePack::GetBorder(TSubclassOf<UCommonBorderStyle> Style)
{
	for (auto itr : Borders)
	{
		if (itr.Get()->GetName() == Style.Get()->GetName())
			return itr;
	}

	return Style;
}

TSubclassOf<UCommonButtonStyle>	FNxStylePack::GetButton(TSubclassOf<UCommonButtonStyle> Style)
{
	for (auto itr : Buttons)
	{
		if (itr.Get()->GetName() == Style.Get()->GetName())
			return itr;
	}

	return Style;
}

TSubclassOf<UCommonTextStyle>	FNxStylePack::GetText(TSubclassOf<UCommonTextStyle> Style)
{
	for (auto itr : Texts)
	{
		if (itr.Get()->GetName() == Style.Get()->GetName())
			return itr;
	}

	return Style;
}

UNxCodeUISettings::UNxCodeUISettings(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{ 
}

#if WITH_EDITOR

FText UNxCodeUISettings::GetSectionText() const
{
	return LOCTEXT("VxUISettingsName", "Plugin VxUI");
}

FText UNxCodeUISettings::GetSectionDescription() const
{
	return LOCTEXT("VxUISettingsDescription", "Kakao-PlugIn VxUI 설정");
}
#endif

#undef LOCTEXT_NAMESPACE