// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/NxTabGroupWidget.h"

#include "NxButtonTab.h"
#include "CommonAnimatedSwitcher.h"

void UNxTabGroupWidget::NativeConstruct() 
{
	Super::NativeConstruct();

	for (auto widget : IncludeWidgets)
	{
		auto tabButton = Cast<UNxButtonTab>(widget.Value);
		if (nullptr != tabButton)
			TabButtons.Add(tabButton);
	}

	//Tab Iink
	for (int i = 0; i < TabButtons.Num(); i++)
	{
		//tabButton->OnClicked().AddUObject(this, &ThisClass::OnTableButtonClick);
		//TabButtons[i]->OnClicked().AddLambda(FOnClicked::CreateLambda([this, i]()
		//{
		//	OnTableButtonClick(i);
		//	return FReply::Handled();
		//}));

		TabButtons[i]->OnClicked().AddLambda([this, i]() {
			OnTableButtonClick(i);
		});
	}
}

void UNxTabGroupWidget::OnTableButtonClick(uint8 InTabPos)
{
	SetTab(InTabPos);
}

void UNxTabGroupWidget::SetTab(uint8 InTabPos, bool InButtonEvent)
{
	if (TabPos == InTabPos)
		return;

	if (InTabPos >= TabButtons.Num())
		InTabPos = TabButtons.Num() - 1;

	//todo
	for (int i = 0; i < TabButtons.Num(); i++)
	{
		if (i == InTabPos)
		{
			if (true == InButtonEvent)
			{
				TabButtons[i]->SetIsEnabled(false);
			}
		}
		else
		{
			TabButtons[i]->SetIsEnabled(true);
		}
	}

	if(nullptr != Switcher)
		Switcher->SetActiveWidgetIndex(InTabPos);

	TabPos = InTabPos;
}

uint8	UNxTabGroupWidget::GetTabPos(UNxButtonTab* ButtonTab)
{
	for (int i = 0; i < TabButtons.Num(); i++)
	{
		if (ButtonTab == TabButtons[i])
			return i;
	}
	return TabButtons.Num();
}