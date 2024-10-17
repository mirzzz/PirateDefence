// Fill out your copyright notice in the Description page of Project Settings.
#include "NxButtonTab.h"

#include "CommonLazyImage.h"
#include "CommonTextBlock.h"
#include "NxCodeUIUtility.h"

void UNxButtonTab::NativePreConstruct()
{
	Super::NativePreConstruct();

	for (auto& text : Text_Names)
		text->SetStyle(GetCurrentTextStyleClass());
}

void UNxButtonTab::NativeConstruct()
{
	Super::NativeConstruct();

	// 아트에서 설정하고 오게 하자 ~
	//SetIsSelectable(true);
	//SetIsToggleable(false);
}

void UNxButtonTab::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();

	for (auto& text : Text_Names)
		text->SetStyle(GetCurrentTextStyleClass());
}

void UNxButtonTab::UpdateInputActionWidget()
{
	Super::UpdateInputActionWidget();
}

void UNxButtonTab::CollectionChild()
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

void UNxButtonTab::RefreshButtonIcon()
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

void UNxButtonTab::RefreshButtonText()
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

void UNxButtonTab::Init(uint8 InTabId, bool InSelecte)
{
	TabId = InTabId;

	if (true == InSelecte)
		Selecte();
}

void UNxButtonTab::Selecte()
{
	SetIsSelected(true);
}

void UNxButtonTab::Unselecte()
{
	ClearSelection();
}