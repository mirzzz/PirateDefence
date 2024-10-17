// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Frame/NxBorderBase.h"
#include "NxTabGroup.generated.h"

class UNxButtonTab;
class UCommonAnimatedSwitcher;
class UVxUIBaseActivatableWidget;

/**
 * 
 */
UCLASS(Config = NxCodeUI, DefaultConfig, ClassGroup = UI, meta = (Category = CodeUI, DisplayName = "NxTabGroup"))
class NXCODEUI_API UNxTabGroup : public UNxBorderBase
{
	GENERATED_BODY()

public:
	virtual void PostLoad() override;
	virtual void OnInitializedWidget(UNxActivatableWidgetBase* InTopWidget);

	virtual void OnButtonSelect(uint8 InTabPos, bool InSelect);

	virtual void SelectedTab(uint8 InTabPos);

	void	SelectTab(uint8 InTabPos);
	uint8	GetTabPos(UNxButtonTab* ButtonTab);

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabGroupSelect, uint8, PreTabPos, uint8, TabPos);
	FOnTabGroupSelect		OnTabGroupSelect;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widget)
	uint8 TabPos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widget)
	FString SwitcherName;

	TObjectPtr<UCommonAnimatedSwitcher> FindSwitcher;
	TObjectPtr<UCommonAnimatedSwitcher> ChildSwitcher;

	TArray<TObjectPtr<UNxButtonTab>> TabButtons;
};
