// Fill out your copyright notice in the Description page of Project Settings.
#include "NxButtonNormal.h"

#include "CommonLazyImage.h"
#include "CommonTextBlock.h"

#include "NxCodeUIUtility.h"

void UNxButtonNormal::NativePreConstruct()
{
	Super::NativePreConstruct();

	for (auto& text : Text_Names)
		text->SetStyle(GetCurrentTextStyleClass());
}

void UNxButtonNormal::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNxButtonNormal::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();

	for (auto& text : Text_Names)
		text->SetStyle(GetCurrentTextStyleClass());
}

void UNxButtonNormal::UpdateInputActionWidget()
{
	Super::UpdateInputActionWidget();
}

void UNxButtonNormal::CollectionChild()
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

void UNxButtonNormal::RefreshButtonIcon()
{
	if (bOverride_ButtonIcon && Image_Icons.Num() == ButtonIcons.Num())
	{
		for(int i=0; i< Image_Icons.Num(); i++)
		{
			Image_Icons[i]->SetBrush(ButtonIcons[i].Brush);
			Nx_UtilityUI::SetPadding(Image_Icons[i], ButtonIcons[i].Padding);
		}
	}
}

void UNxButtonNormal::RefreshButtonText()
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