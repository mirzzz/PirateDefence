// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonBorder.h"
#include "NxBorderBase.generated.h"

class USizeBox;
class UNxActivatableWidgetBase;

/**
 * 
 */
UCLASS(Abstract)

class NXCODEUI_API UNxBorderBase : public UCommonBorder
{
	GENERATED_BODY()

public:
	virtual void PostLoad() override;
	virtual void OnInitializedWidget(UNxActivatableWidgetBase* InTopWidget);

	void GetAllWidgets(TArray<UWidget*>& Widgets) const;
	void ForEachWidget(TFunctionRef<void(UWidget*)> Predicate) const;

protected:
	virtual void CollectionChild();

protected:
	UNxActivatableWidgetBase* TopWidget;
	TMap<FName, UWidget*>		IncludeWidgets;

protected:
};
