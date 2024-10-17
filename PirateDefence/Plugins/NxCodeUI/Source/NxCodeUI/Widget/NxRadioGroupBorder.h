// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Frame/NxBorderBase.h"
#include "NxRadioGroupBorder.generated.h"

class UNxButtonToggle;
class UCommonAnimatedSwitcher;
class UVxUIBaseActivatableWidget;

/**
 * 
 */
UCLASS(Config = NxCodeUI, DefaultConfig, ClassGroup = UI, meta = (Category = CodeUI, DisplayName = "NxRadioGroupBorder"))
class NXCODEUI_API UNxRadioGroupBorder : public UNxBorderBase
{
	GENERATED_BODY()

public:
	virtual void PostLoad() override;
	virtual void OnInitializedWidget(UNxActivatableWidgetBase* InTopWidget);

	virtual void OnButtonClick(uint8 InTabPos);

	virtual void SelectButton(uint8 InTabPos, bool InButtonEvent = false);

	uint8	GetSelectPos(UNxButtonToggle* Button);

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGroupSelect, uint8, PreTabPos, uint8, TabPos);
	FOnGroupSelect		OnGroupSelect;

protected:
	UPROPERTY(BlueprintReadOnly, Category = Widget)
	uint8 SelectPos;

	TArray<TObjectPtr<UNxButtonToggle>> Buttons;
};
