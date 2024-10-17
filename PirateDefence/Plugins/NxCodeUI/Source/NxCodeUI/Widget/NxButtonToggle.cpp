// Fill out your copyright notice in the Description page of Project Settings.


#include "NxButtonToggle.h"

#include "CommonLazyImage.h"
#include "CommonTextBlock.h"

#include "NxCodeUIUtility.h"

void UNxButtonToggle::NativePreConstruct()
{
	Super::NativePreConstruct();

	for (auto& text : Text_Names)
		text->SetStyle(GetCurrentTextStyleClass());
}

void UNxButtonToggle::NativeConstruct()
{
	Super::NativeConstruct();

	//아트에서 설정하고 오게 하자 ~
	//SetIsSelectable(true);
	//SetIsToggleable(true);
}

void UNxButtonToggle::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();

	for (auto& text : Text_Names)
		text->SetStyle(GetCurrentTextStyleClass());
}

void UNxButtonToggle::UpdateInputActionWidget()
{
	Super::UpdateInputActionWidget();
}

void UNxButtonToggle::CollectionChild()
{
	Super::CollectionChild();

	for (auto& widget : IncludeWidgets)
	{
		auto image = Cast<UCommonLazyImage>(widget.Value);
		if (nullptr != image)
			Image_Icons.Add(image);

		auto text = Cast<UCommonTextBlock>(widget.Value);
		if (nullptr != text)
			Text_Names.Add(text);
	}
}

void UNxButtonToggle::RefreshButtonIcon()
{
	if (bOverride_ButtonIcon && Image_Icons.Num() == ButtonIcons.Num())
	{
		for (int i = 0; i < Image_Icons.Num(); i++)
		{
			Image_Icons[i]->SetBrush(ButtonIcons[i].Brush);
			Nx_UtilityUI::SetPadding(Image_Icons[i], ButtonIcons[i].Padding);
		}
	}
}

void UNxButtonToggle::RefreshButtonText()
{
	if (bOverride_ButtonText && Text_Names.Num() == ButtonTexts.Num())
	{
		for (int i = 0; i < Text_Names.Num(); i++)
		{
			Text_Names[i]->SetText(ButtonTexts[i].Text);
			Nx_UtilityUI::SetPadding(Text_Names[i], ButtonTexts[i].Padding);
		}
	}
}

void UNxButtonToggle::Init(uint8 InTabId, bool InSelected)
{
	TabId = InTabId;

	if (true == InSelected)
		Toggle();
}

void UNxButtonToggle::Toggle()
{
	SetIsSelected(true);
}