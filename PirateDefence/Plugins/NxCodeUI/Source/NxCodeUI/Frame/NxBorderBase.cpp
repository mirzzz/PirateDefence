// Fill out your copyright notice in the Description page of Project Settings.


#include "Frame/NxBorderBase.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Animation/UMGSequencePlayer.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanel.h"

#include "Misc/QualifiedFrameTime.h"
#include "MovieScene.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"

#include "NxActivatableWidgetBase.h"

void UNxBorderBase::PostLoad()
{
	Super::PostLoad();

	CollectionChild();
}

void UNxBorderBase::OnInitializedWidget(UNxActivatableWidgetBase* InTopWidget)
{
	TopWidget = InTopWidget;

	CollectionChild();
}

void UNxBorderBase::CollectionChild()
{
	// Widget 등록.
	IncludeWidgets.Empty();

	TArray<UWidget*> childWidgets = GetAllChildren();
	for (auto& comp : childWidgets)
	{
		UWidget* widget = comp;
		if (widget == nullptr || widget->IsValidLowLevel() == false)
			continue;

		FString  widgetString = widget->GetName();
		IncludeWidgets.Add(FName(*widgetString), widget);
	}

	TArray<UWidget*> widgets;
	GetAllWidgets(widgets);
	for (auto widget : widgets)
	{
		IncludeWidgets.Add(FName(*widget->GetName()), widget);
	}
}

void UNxBorderBase::GetAllWidgets(TArray<UWidget*>& Widgets) const
{
	ForEachWidget([&Widgets](UWidget* Widget) {
		Widgets.Add(Widget);
		});
}

void UNxBorderBase::ForEachWidget(TFunctionRef<void(UWidget*)> Predicate) const
{
	//Predicate(this);
	for (auto itr : IncludeWidgets)
	{
		// Start with the root widget.
		UWidgetTree::ForWidgetAndChildren(itr.Value, Predicate);
	}
}