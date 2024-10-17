// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/NxTabGroup.h"

#include "NxButtonTab.h"
#include "CommonAnimatedSwitcher.h"
#include "Frame/NxActivatableWidgetBase.h"
#include "Frame/NxActivatableWidgetComponent.h"

void UNxTabGroup::PostLoad()
{
	Super::PostLoad();

	TabButtons.Empty();
	for (auto widget : IncludeWidgets)
	{
		auto tabButton = Cast<UNxButtonTab>(widget.Value);
		if (nullptr != tabButton)
			TabButtons.Add(tabButton);

		auto switcher = Cast<UCommonAnimatedSwitcher>(widget.Value);
		if (nullptr != switcher)
			ChildSwitcher = switcher;
	}

	////Tab Iink
	//for (int i = 0; i < TabButtons.Num(); i++)
	//{
	//	TabButtons[i]->SetIsEnabled(false);
	//	TabButtons[i]->OnClicked().AddLambda([this, i]() {
	//		OnButtonClick(i);
	//	});
	//}
	//SetTab(TabPos, true);
}

void UNxTabGroup::OnInitializedWidget(UNxActivatableWidgetBase* InTopWidget)
{
	Super::OnInitializedWidget(InTopWidget);

	for (auto tabButton : TabButtons)
		tabButton->OnIsSelectedChanged().Clear();
	TabButtons.Empty();
	for (auto widget : IncludeWidgets)
	{
		auto tabButton = Cast<UNxButtonTab>(widget.Value);
		if (nullptr != tabButton)
			TabButtons.Add(tabButton);

		auto switcher = Cast<UCommonAnimatedSwitcher>(widget.Value);
		if (nullptr != switcher)
			ChildSwitcher = switcher;
	}

	if (false == SwitcherName.IsEmpty())
		FindSwitcher = InTopWidget->GetWidget<UCommonAnimatedSwitcher>(SwitcherName);

	//Tab Iink
	for (int i = 0; i < TabButtons.Num(); i++)
	{
		//TabButtons[i]->OnClicked().AddLambda([this, i]() {
		//	OnButtonClick(i);
		//	});

		TabButtons[i]->Init(i, i == TabPos ? true : false);
		TabButtons[i]->OnIsSelectedChanged().AddLambda([this, i](bool select) {
			OnButtonSelect(i, select);
			});
	}
}

void UNxTabGroup::OnButtonSelect(uint8 InTabPos, bool InSelect)
{
	if(true == InSelect)
		SelectedTab(InTabPos);
}

void UNxTabGroup::SelectedTab(uint8 InTabPos)
{
	if (TabPos == InTabPos)
		return;

	if (InTabPos >= TabButtons.Num())
		InTabPos = TabButtons.Num() - 1;

	//todo
	for (int i = 0; i < TabButtons.Num(); i++)
	{
		if (i != InTabPos)
		{
			if (TabButtons[i]->GetSelected())
			{
				TabButtons[i]->Unselecte();
			}
		}
	}

	if (nullptr != FindSwitcher)
	{
		FindSwitcher->SetActiveWidgetIndex(InTabPos);
		auto activeComponent = Cast<UNxActivatableWidgetComponent>(FindSwitcher->GetWidgetAtIndex(InTabPos));
		if (nullptr != activeComponent)
			activeComponent->UpdateControl();
	}

	if (nullptr != ChildSwitcher)
	{
		ChildSwitcher->SetActiveWidgetIndex(InTabPos);
		auto activeComponent = Cast<UNxActivatableWidgetComponent>(ChildSwitcher->GetWidgetAtIndex(InTabPos));
		if (nullptr != activeComponent)
			activeComponent->UpdateControl();
	}

	if (OnTabGroupSelect.IsBound())
		OnTabGroupSelect.Broadcast(TabPos, InTabPos);

	TabPos = InTabPos;
}

void	UNxTabGroup::SelectTab(uint8 InTabPos)
{
	TabButtons[InTabPos]->Selecte();
}


uint8	UNxTabGroup::GetTabPos(UNxButtonTab* ButtonTab)
{
	for (int i = 0; i < TabButtons.Num(); i++)
	{
		if (ButtonTab == TabButtons[i])
			return i;
	}
	return TabButtons.Num();
}