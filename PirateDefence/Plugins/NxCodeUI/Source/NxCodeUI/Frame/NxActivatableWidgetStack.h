// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "NxActivatableWidgetWindow.h"
#include "NxActivatableWidgetStack.generated.h"

/**
 * 
 */
UCLASS(meta = (DisableNativeTick))
class NXCODEUI_API UNxActivatableWidgetStack : public UCommonActivatableWidgetStack
{
	GENERATED_BODY()

public:
	UNxActivatableWidgetStack(const FObjectInitializer& ObjectInitializer);

	virtual void RemoveFromParent() override;

	TArray<TObjectPtr<UNxActivatableWidgetWindow>>	GetWnds();

	void SetShow(bool show);
};
