// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/NxRadioGroupBorder.h"

#include "NxButtonToggle.h"
#include "CommonAnimatedSwitcher.h"
#include "Frame/NxActivatableWidgetBase.h"

void UNxRadioGroupBorder::PostLoad()
{
	Super::PostLoad();

	Buttons.Empty();
	for (auto widget : IncludeWidgets)
	{
		auto button = Cast<UNxButtonToggle>(widget.Value);
		if (nullptr != button)
			Buttons.Add(button);
	}

	//Tab Iink
	for (int i = 0; i < Buttons.Num(); i++)
	{
		Buttons[i]->OnClicked().AddLambda([this, i]() {
			OnButtonClick(i);
		});
	}
}

void UNxRadioGroupBorder::OnInitializedWidget(UNxActivatableWidgetBase* InTopWidget)
{
	Super::OnInitializedWidget(InTopWidget);

	for (auto button : Buttons)
		button->OnClicked().Clear();
	Buttons.Empty();
	for (auto widget : IncludeWidgets)
	{
		auto button = Cast<UNxButtonToggle>(widget.Value);
		if (nullptr != button)
			Buttons.Add(button);
	}

	//Tab Iink
	for (int i = 0; i < Buttons.Num(); i++)
	{
		Buttons[i]->OnClicked().AddLambda([this, i]() {
			OnButtonClick(i);
			});
	}
}

void UNxRadioGroupBorder::OnButtonClick(uint8 InSelectPos)
{
	SelectButton(InSelectPos);
}

void UNxRadioGroupBorder::SelectButton(uint8 InSelectPos, bool InButtonEvent)
{
	if (SelectPos == InSelectPos)
		return;

	if (InSelectPos >= Buttons.Num())
		InSelectPos = Buttons.Num() - 1;

	//todo
	for (int i = 0; i < Buttons.Num(); i++)
	{
		if (i == InSelectPos)
		{
			if (true == InButtonEvent)
			{
				Buttons[i]->SetIsEnabled(false);
			}
		}
		else
		{
			Buttons[i]->SetIsEnabled(true);
		}
	}

	if (OnGroupSelect.IsBound())
		OnGroupSelect.Broadcast(SelectPos, InSelectPos);

	SelectPos = InSelectPos;
}

uint8	UNxRadioGroupBorder::GetSelectPos(UNxButtonToggle* Button)
{
	for (int i = 0; i < Buttons.Num(); i++)
	{
		if (Button == Buttons[i])
			return i;
	}
	return Buttons.Num();
}