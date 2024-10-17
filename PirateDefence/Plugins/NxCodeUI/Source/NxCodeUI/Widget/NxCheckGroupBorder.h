// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Frame/NxBorderBase.h"
#include "NxCheckGroupBorder.generated.h"

class UNxButtonToggle;
class UCommonAnimatedSwitcher;
class UVxUIBaseActivatableWidget;

/**
 * 
 */
UCLASS(Config = NxCodeUI, DefaultConfig, ClassGroup = UI, meta = (Category = CodeUI, DisplayName = "NxCheckGroupBorder"))
class NXCODEUI_API UNxCheckGroupBorder : public UNxBorderBase
{
	GENERATED_BODY()

public:
	virtual void PostLoad() override;
	virtual void OnInitializedWidget(UNxActivatableWidgetBase* InTopWidget);

	virtual void OnButtonClick(uint8 InTabPos, bool InSelected);
	virtual void SelectButton(uint8 InTabPos, bool InSelected);

	TArray<uint8> GetSelectedButtons();

	uint8	GetSelectPos(UNxButtonToggle* Button);

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGroupSelect, uint8, ButtonPos, bool, IsSelected);
	FOnGroupSelect		OnGroupSelect;

protected:
	//UPROPERTY(EditAnywhere, Category = Widget, meta = (InlineEditConditionToggle))
	//uint8 bOverride_CountMin : 1;

	//UPROPERTY(EditAnywhere, Category = Widget, meta = (ClampMin = "1", editcondition = "bOverride_CountMin"))
	//uint8 CheckCountMin;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (InlineEditConditionToggle))
	uint8 bOverride_CountMax : 1;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (ClampMin = "1", editcondition = "bOverride_CountMax"))
	uint8 CheckCountMax;

	//UPROPERTY(BlueprintReadOnly, Category = Widget)
	TArray<uint8> SelectPoses;

	TArray<TObjectPtr<UNxButtonToggle>> Buttons;
};
