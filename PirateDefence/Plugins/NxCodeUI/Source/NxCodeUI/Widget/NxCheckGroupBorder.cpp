// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/NxCheckGroupBorder.h"

#include "NxButtonToggle.h"
#include "CommonAnimatedSwitcher.h"
#include "Frame/NxActivatableWidgetBase.h"

void UNxCheckGroupBorder::PostLoad()
{
	Super::PostLoad();

	Buttons.Empty();
	for (auto widget : IncludeWidgets)
	{
		auto button = Cast<UNxButtonToggle>(widget.Value);
		if (nullptr != button)
			Buttons.Add(button);
	}

	////Tab Iink
	//for (int i = 0; i < Buttons.Num(); i++)
	//{
	//	Buttons[i]->OnClicked().AddLambda([this, i]() {
	//		OnButtonClick(i);
	//		});
	//}
}

void UNxCheckGroupBorder::OnInitializedWidget(UNxActivatableWidgetBase* InTopWidget)
{
	Super::OnInitializedWidget(InTopWidget);

	for (auto button : Buttons)
		button->OnIsSelectedChanged().Clear();
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
		Buttons[i]->Init(i, false);
		Buttons[i]->OnIsSelectedChanged().AddLambda([this, i](bool Selected) {
			OnButtonClick(i, Selected);
			});
	}
}

void UNxCheckGroupBorder::OnButtonClick(uint8 InSelectPos, bool InSelected)
{
	SelectButton(InSelectPos, InSelected);
}

void UNxCheckGroupBorder::SelectButton(uint8 InSelectPos, bool InSelected)
{
	TArray<uint8> selectedList = GetSelectedButtons();

	if (true == InSelected)
	{
		if (bOverride_CountMax)
		{
			if (selectedList.Num() >= CheckCountMax)
			{
				for (int i = 0; i < Buttons.Num(); i++)
				{
					if (false == Buttons[i]->GetSelected())
						Buttons[i]->SetIsInteractionEnabled(false);
				}
			}
		}
	}
	else
	{
		if (bOverride_CountMax)
		{
			for (int i = 0; i < Buttons.Num(); i++)
			{
				if (false == Buttons[i]->GetSelected())
					Buttons[i]->SetIsInteractionEnabled(true);
			}
		}
	}

	if (OnGroupSelect.IsBound())
		OnGroupSelect.Broadcast(InSelectPos, InSelected);
}

TArray<uint8> UNxCheckGroupBorder::GetSelectedButtons()
{
	TArray<uint8>	checkList;
	for (int i = 0; i < Buttons.Num(); i++)
	{
		if (Buttons[i]->GetSelected())
			checkList.Add(i);
	}
	return checkList;
}

uint8	UNxCheckGroupBorder::GetSelectPos(UNxButtonToggle* Button)
{
	for (int i = 0; i < Buttons.Num(); i++)
	{
		if (Button == Buttons[i])
			return i;
	}
	return Buttons.Num();
}