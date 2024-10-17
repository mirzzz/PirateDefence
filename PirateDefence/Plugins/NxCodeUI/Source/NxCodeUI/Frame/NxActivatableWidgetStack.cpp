// Fill out your copyright notice in the Description page of Project Settings.
#include "NxActivatableWidgetStack.h"

#include "CommonActivatableWidget.h"

UNxActivatableWidgetStack::UNxActivatableWidgetStack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UNxActivatableWidgetStack::RemoveFromParent()
{

	Super::RemoveFromParent();
}

TArray<TObjectPtr<UNxActivatableWidgetWindow>>	UNxActivatableWidgetStack::GetWnds()
{
	TArray<TObjectPtr<UNxActivatableWidgetWindow>> wnds;
	const TArray<UCommonActivatableWidget*>& widgets = GetWidgetList();
	for (auto& widget : widgets)
	{
		UNxActivatableWidgetWindow* wnd = Cast<UNxActivatableWidgetWindow>(widget);
		if (nullptr != wnd)
			wnds.Add(wnd);
	}

	return wnds;
}


void UNxActivatableWidgetStack::SetShow(bool show)
{
	for (auto& wnd : GetWnds())
	{
		if (show)
		{
			wnd->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			wnd->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
